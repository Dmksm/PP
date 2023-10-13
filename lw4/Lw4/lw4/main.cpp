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
#include <sstream>

enum class Priority
{
    AboveNormal,
    Normal,
    BelowNormal
};

struct InputOutputImages
{
    cv::Mat* input;
    cv::Mat* output;
    unsigned threadNumber;
    unsigned threadsNumber;
    std::chrono::steady_clock::time_point startTime;
};

DWORD WINAPI ThreadProc(const LPVOID lpParam)
{
    cv::Mat* image = (*(InputOutputImages*)lpParam).input;
    cv::Mat* copyImage = (*(InputOutputImages*)lpParam).output;
    unsigned threadNumber = (*(InputOutputImages*)lpParam).threadNumber;
    unsigned threadsNumber = (*(InputOutputImages*)lpParam).threadsNumber;
    const auto startTime = (*(InputOutputImages*)lpParam).startTime;

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

    std::stringstream fileNameStream;
    fileNameStream << "timeTrack#" << threadNumber << ".txt";
    std::ofstream file(fileNameStream.str());
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

            const auto duration = std::chrono::steady_clock::now() - startTime;
            double timeMarker = std::chrono::duration<double>(duration).count();
            file << timeMarker << std::endl;

        }
    }
    ExitThread(0);
}

void PrintInvalidArgsMsg()
{
    std::cout << "Error! Given parametes must be <program.exe> <input file name> <output file name>"
        << " <threads number> <cores number> <priority #1> ... <priority #(threads number - 1)>" << std::endl;
}

int main(int argv, char* argc[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);


    unsigned threadsNumber;
    if (argv < 5)
    {
        PrintInvalidArgsMsg();
        return 1;
    }
    else
    {
        threadsNumber = std::stoul(argc[3]);
        if (argv != 5 + threadsNumber)
        {
            PrintInvalidArgsMsg();
            return 1;
        }
    }

    std::string inputFileName = argc[1], outputFileName = argc[2];

    const auto startTime = std::chrono::steady_clock::now();

    std::ifstream inputFile(inputFileName, std::ios::in | std::ios::binary);
    std::ofstream outputFile(outputFileName);
    unsigned coresNumber = std::stoul(argc[4]);

    if (!(inputFile.is_open() &&
        threadsNumber >= 1 && threadsNumber <= 3 && coresNumber >= 1 && coresNumber <= 4))
    {
        std::cout << "Error! filename must be name for existed "
            << " Thread number must be between 1 and 3 and cores number must be between 1 and 4!"
            << " Given input and output file names: " << inputFileName << " and "
            << outputFileName << " . And given threads number and cores number: " << threadsNumber
            << " and " << coresNumber << std::endl;
        return 1;
    }

    std::vector<DWORD_PTR> affinity{ 1, 3, 7, 15 };
    DWORD_PTR processAffinityMask = affinity[coresNumber - 1];


    cv::Mat image = cv::imread(inputFileName);
    cv::Mat copyImage;

    image.copyTo(copyImage);

    HANDLE* handles = new HANDLE[threadsNumber];

    BOOL success = SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)processAffinityMask);
    int v[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    std::vector<InputOutputImages> args;
    for (int threadNumber = 0; threadNumber < threadsNumber; threadNumber++)
    {
        args.emplace_back(&image, &copyImage, v[threadNumber], threadsNumber, startTime);
    }

    std::vector<DWORD> priorities;
    for (int threadNumber = 0; threadNumber < threadsNumber; threadNumber++)
    {
        std::string priorityName = argc[threadNumber + 5];
        int priority;
        if (priorityName == "normal")
        {
            priority = THREAD_PRIORITY_NORMAL;
        }
        else if (priorityName == "above_normal")
        {
            priority = THREAD_PRIORITY_ABOVE_NORMAL;
        }
        else if (priorityName == "below_normal")
        {
            priority = THREAD_PRIORITY_BELOW_NORMAL;
        }
        else
        {
            std::cout << "Error! Unknown priority. Given " << priorityName << std::endl;
            return 1;
        }
        priorities.emplace_back(priority);
    }

    for (int threadNumber = 0; threadNumber < threadsNumber; threadNumber++)
    {
        HANDLE handle = CreateThread(NULL, 0, &ThreadProc,
            (LPVOID)&args[threadNumber], CREATE_SUSPENDED, NULL);
        if (!SetThreadPriority(handle, priorities[threadNumber]))
        {
            DWORD err = GetLastError();
            std::cout << "set failed! Code " << err;
        }
        else
        {
            std::cout << "set success!";
        }
        handles[threadNumber] = handle;

        ResumeThread(handles[threadNumber]);
    }

    WaitForMultipleObjects(threadsNumber, handles, true, INFINITE);
    cv::imwrite(outputFileName, copyImage);

    return 0;
}
