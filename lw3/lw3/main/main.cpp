#include <iostream>
#include <windows.h>
#include <string>
#include <algorithm>
#include <time.h>
#include <chrono>

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
    const int ARRAY_SIZE = 10;
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
};

DWORD WINAPI ThreadProc(const LPVOID lpParam)
{
    const int threadNumber = ((Args*)lpParam)->number;
    printf("Поток №%d выполняет свою работу\n", threadNumber);
    const auto start_time = std::chrono::steady_clock::now();
    CreateSortedRandomArray();
    const auto duration = std::chrono::steady_clock::now() - start_time;
    std::cout << std::chrono::duration<double>(duration).count() << " seconds" << std::endl;
    ExitThread(0);
}

int main(int argv, char* argc[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int n = std::stoi(argc[1]);
    HANDLE* handles = new HANDLE[2];
    Args args[2];
    for (int i = 0; i < 2; i++)
    {
        Args in;
        in.number = i + 1;
        args[i] = in;
    }

    for (int i = 0; i < 2; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)&(args[i]), NULL, NULL);
    }

    WaitForMultipleObjects(n, handles, true, INFINITE);
    return 0;
}