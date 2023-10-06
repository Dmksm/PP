#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <windows.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <math.h>
#include <numbers>       
#include <bitset>   

struct InputOutputFiles
{
    std::ifstream& input;
    std::ofstream& output;
};

struct InputOutputImages
{
    cv::Mat* input;
    cv::Mat* output;
    unsigned threadNumber;
    unsigned threadsNumber;
};

DWORD WINAPI ThreadProc(const LPVOID lpParam)
{
    cv::Mat* image = (*(InputOutputImages*)lpParam).input;
    cv::Mat* copyImage = (*(InputOutputImages*)lpParam).output;
    unsigned threadNumber = (*(InputOutputImages*)lpParam).threadNumber;
    unsigned threadsNumber = (*(InputOutputImages*)lpParam).threadsNumber;

    double radius = 2;

    unsigned width = ((*image).size().width / threadsNumber);
    double maxX = ((*image).size().width - radius), maxY = ((*image).size().height - radius);
    double startX = radius, startY = radius;
    startX += width * threadNumber;
    maxX = std::min(startX + width, maxX);


    double sigma = std::max(radius / 2, (double)1);
    const double kernelWidth = (2 * radius) + 1;
    std::vector<std::vector<double>> kernel(
        kernelWidth,
        std::vector<double>(kernelWidth));

    double sum = 0.0;
    for (double x = -radius; x < radius; x++)
    {
        for (double y = -radius; y < radius; y++)
        {
            double exponentNumerator = (-(x * x + y * y));
            double exponentDenominator = (2 * sigma * sigma);

            double eExpression = std::pow(std::exp(1), exponentNumerator / exponentDenominator);
            double kernelValue = (eExpression / (2 * std::numbers::pi * sigma * sigma));

            // We add radius to the indices to prevent out of bound issues because x and y can be negative
            kernel[x + radius][y + radius] = kernelValue;
            sum += kernelValue;
        }
    }
    // Normalize the kernel
    // This ensures that all of the values in the kernel together add up to 1
    for (double x = 0; x < kernelWidth; x++)
    {
        for (double y = 0; y < kernelWidth; y++)
        {
            kernel[x][y] /= sum;
        }
    }

    for (double x = startX; x < maxX; x++)
    {
        for (double y = startY; y < maxY; y++)
        {
            double redValue = 0.0;
            double greenValue = 0.0;
            double blueValue = 0.0;

            // This is the convolution step
            // We run the kernel over this grouping of pixels centered around the pixel at (x,y)
            for (double kernelX = -radius; kernelX < radius; kernelX++)
            {
                for (double kernelY = -radius; kernelY < radius; kernelY++)
                {
                    // Load the weight for this pixel from the convolution matrix
                    double kernelValue = kernel[kernelX + radius][kernelY + radius];

                    // Multiply each channel by the weight of the pixel as specified by the kernel
                    redValue += (*image).at<cv::Vec3b>(y - kernelY, x - kernelX)[0] * kernelValue;
                    greenValue += (*image).at<cv::Vec3b>(y - kernelY, x - kernelX)[1] * kernelValue;
                    blueValue += (*image).at<cv::Vec3b>(y - kernelY, x - kernelX)[2] * kernelValue;
                }

            }
            // New RGB value for output image at position (x,y)
            (*copyImage).at<cv::Vec3b>(y, x)[0] = (UINT8)redValue;
            (*copyImage).at<cv::Vec3b>(y, x)[1] = (UINT8)greenValue;
            (*copyImage).at<cv::Vec3b>(y, x)[2] = (UINT8)blueValue;
        }
    }
    ExitThread(0);
}

int main(int argv, char* argc[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    const auto start_time = std::chrono::steady_clock::now();
    if (argv != 5)
    {
        std::cout << "Error! Given parametes must be <program.exe> <input file name> <output file name>"
            << " <execution threads number> <cores number>" << std::endl;
        return 1;
    }

    std::string inputFileName = argc[1], outputFileName = argc[2];

    std::ifstream inputFile(inputFileName, std::ios::in | std::ios::binary);
    std::ofstream outputFile(outputFileName);
    unsigned threadsNumber = std::stoul(argc[3]);
    unsigned coresNumber = std::stoul(argc[4]);
    if (!(inputFile.is_open() &&
        threadsNumber >= 1 && threadsNumber <= 16 && coresNumber >= 1 && coresNumber <= 4))
    {
        std::cout << "Error! filename must be name for existed "
            << " Thread number must be between 1 and 16 and cores number must be between 1 and 4!"  
            << " Given input and output file names: " << inputFileName << " and "
            << outputFileName << " . And given threads number and cores number: " << threadsNumber 
            << " and " << coresNumber << std::endl;
        return 1;
    }

    cv::Mat image = cv::imread(inputFileName);
    cv::Mat copyImage;

    std::cout << " seconds " << std::endl;
    image.copyTo(copyImage);

    HANDLE* handles = new HANDLE[threadsNumber];
    for (int threadNumber = 0; threadNumber < threadsNumber; threadNumber++)
    {
        cv::waitKey(3);
        InputOutputImages inputOutputImages(&image, &copyImage, threadNumber, threadsNumber);
        handles[threadNumber] = CreateThread(NULL, 0, &ThreadProc,
            (LPVOID)&inputOutputImages, CREATE_SUSPENDED, NULL);
        std::string coreMask = "000000000001";
        if (coresNumber == 2)
        {
            coreMask = "000000000011";
        }
        if (coresNumber == 3)
        {
            coreMask = "000000000111";
        }
        if (coresNumber == 4)
        {
            coreMask = "000000001111";
        }

        std::bitset<16> mask(coreMask);
        SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)&mask);
        ResumeThread(handles[threadNumber]);
    }

    WaitForSingleObject(handles, INFINITE);

    unsigned delay = 500;
    cv::waitKey(delay);
    cv::imwrite(outputFileName, copyImage);
    const auto duration = std::chrono::steady_clock::now() - start_time;
    std::cout << std::chrono::duration<double>(duration).count() << " seconds" << std::endl;

    cv::imshow("Original", image);
    cv::imshow("Gaussian Copy imgage", copyImage);

    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
