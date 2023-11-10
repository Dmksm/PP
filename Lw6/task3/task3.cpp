#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <cmath>
#include <algorithm>
#include <math.h>

unsigned GetNumberOfDigits(unsigned i)
{
    return i > 0 ? (int)log10((double)i) + 1 : 1;
}

void RandomiseMatrix(int** matrix, int n, int m) 
{
    for (int i = 0; i < n; i++) 
    {
        for (int j = 0; j < m; j++) 
        {
            matrix[i][j] = rand() % 11;
        }
    }
}

void PrintSpaces(int digitsNumber)
{
    while (digitsNumber > 0)
    {
        std::cout << " ";
        --digitsNumber;
    }
}

void PrintMatrix(int** matrix, int n, int m, int maxSpacesCount)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            std::cout << matrix[i][j];
            int digitsNumber = maxSpacesCount - GetNumberOfDigits(matrix[i][j]) + 2;
            PrintSpaces(digitsNumber);
        }
        std::cout << std::endl;
    }
}

void ParallelMatrixMultiplication(
    int**& resultMaxtrix, 
    int** matrix1, 
    int n1, 
    int m1, 
    int** matrix2, 
    int n2, 
    int m2, 
    int& maxSpacesCount
)
{
    const int THREADS_NUMBER = 2;
    omp_set_num_threads(THREADS_NUMBER);
    int i, j, k;
    #pragma omp parallel for shared(matrix1, matrix2, resultMaxtrix) private(i, j, k)
    for (i = 0; i < n1; i++) 
    {
        for (j = 0; j < m2; j++) 
        {
            resultMaxtrix[i][j] = 0;
            for (k = 0; k < m1; k++) 
            {
                int newValue = (matrix1[i][k] * matrix2[k][j]);
                resultMaxtrix[i][j] += newValue;
                maxSpacesCount = std::max(maxSpacesCount, (int)GetNumberOfDigits(newValue));
            }
        }
    }
}

void MallocMemory(int**& maxtrix, int n, int m)
{
    maxtrix = (int**)malloc(sizeof(int*) * n);
    for (int i = 0; i < n; i++)
    {
        maxtrix[i] = (int*)malloc(sizeof(int) * m);
    }
}

int main() 
{
    srand(time(NULL));

    int n1 = 3;
    int m1 = 3;
    int** matrix1;
    MallocMemory(matrix1, n1, m1);
    RandomiseMatrix(matrix1, n1, m1);

    int n2 = 3;
    int m2 = 3;
    int** matrix2;
    MallocMemory(matrix2, n2, m2);
    RandomiseMatrix(matrix2, n2, m2);

    int** resultMaxtrix;
    MallocMemory(resultMaxtrix, n1, m2);
    
    int maxSpacesCount;
    ParallelMatrixMultiplication(resultMaxtrix, matrix1, n1, m1, matrix2, n2, m2, maxSpacesCount);
    PrintMatrix(matrix1, n1, m1, maxSpacesCount);
    std::cout << std::endl;
    PrintMatrix(matrix2, n2, m2, maxSpacesCount);
    std::cout << std::endl;
    PrintMatrix(resultMaxtrix, n1, m2, maxSpacesCount);

    return 0;
}