#include <iostream>
#include <windows.h>
#include <string>
#include <algorithm>
#include <time.h>
#include <chrono>
#include <timeapi.h>
#include <fstream>

int partition(int a[], int start, int end)
{
    // Pick the rightmost element as a pivot from the array
    int pivot = a[end];

    // elements less than the pivot will be pushed to the left of `pIndex`
    // elements more than the pivot will be pushed to the right of `pIndex`
    // equal elements can go either way
    int pIndex = start;

    // each time we find an element less than or equal to the pivot, `pIndex`
    // is incremented, and that element would be placed before the pivot.
    for (int i = start; i < end; i++)
    {
        if (a[i] <= pivot)
        {
            std::swap(a[i], a[pIndex]);
            pIndex++;
        }
    }

    // swap `pIndex` with pivot
    std::swap(a[pIndex], a[end]);

    // return `pIndex` (index of the pivot element)
    return pIndex;
}

void quicksort(int a[], int start, int end)
{
    // base condition
    if (start >= end) {
        return;
    }

    // rearrange elements across pivot
    int pivot = partition(a, start, end);

    // recur on subarray containing elements that are less than the pivot
    quicksort(a, start, pivot - 1);

    // recur on subarray containing elements that are more than the pivot
    quicksort(a, pivot + 1, end);
}

void CreateSortedRandomArray()
{
    const int ARRAY_SIZE = 500;
    int arr[ARRAY_SIZE];
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        int rnumber = rand() % 5;
        arr[i] = rnumber;
    }
    int n = sizeof(arr) / sizeof(arr[0]);
    quicksort(arr, 0, n - 1);
}

struct Args
{
    int number;
    int operationCount;
    std::string outFileName;
    std::chrono::steady_clock::time_point startTime;
};

DWORD WINAPI ThreadProc(const LPVOID lpParam)
{
    const int threadNumber = ((Args*)lpParam)->number;
    int operationCount = ((Args*)lpParam)->operationCount;
    std::ofstream outFile(((Args*)lpParam)->outFileName);

    outFile << "Поток № " << threadNumber << std::endl;

    const auto startTime = ((Args*)lpParam)->startTime;
    while (operationCount)
    {
        CreateSortedRandomArray();
        const auto duration = std::chrono::steady_clock::now() - startTime;
        
        outFile << std::chrono::duration<double>(duration).count() * 1000 << std::endl;

        --operationCount;
    }
   
    ExitThread(0);
}

int main(int argv, char* argc[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argv != 4)
    {
        return 1;
    }
    
    int operationCount = std::stoi(argc[1]);
    HANDLE* handles = new HANDLE[2];
    Args args[2];

    system("pause");
    
    const auto startTime = std::chrono::steady_clock::now();
    for (int i = 0; i < 2; i++)
    {
        Args in;
        in.number = i + 1;
        in.operationCount = operationCount;
        in.outFileName = argc[i + 2];
        in.startTime = startTime;
        args[i] = in;
    }

    for (int i = 0; i < 2; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)&(args[i]), NULL, NULL);
    }

    WaitForMultipleObjects(2, handles, true, INFINITE);
    return 0;
}