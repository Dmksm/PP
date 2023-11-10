#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>
#include <cmath>
#include <chrono>
#include <omp.h>

double SumReduction = 0;
double SumAtomic = 0;
double SumParallel = 0;

double GetValue(int n)
{
	double value = pow(-1, n + 1) / (2 * n - 1);
	return value;
}

void GetAndPrintPiResult(int n)
{
	double sum = 0;
	for (int i = 1; i <= n; i++)
	{
		sum += GetValue(i);
	}

	std::cout << sum * 4 << std::endl;
}

void GetAndPrintPiResultParallel(int n)
{
	#pragma omp parallel for
	for (int i = 1; i <= n; i++)
	{
		SumParallel += GetValue(i);
	}

	std::cout << SumParallel * 4 << std::endl;
}

void GetAndPrintPiResultAtomic(int n)
{
	#pragma omp parallel for
	for (int i = 1; i <= n; i++)
	{
		#pragma omp atomic
		SumAtomic += GetValue(i);
	}

	std::cout << SumAtomic * 4 << std::endl;
}

void GetAndPrintPiResultReduction(int n)
{
	#pragma omp for reduction(+ : SumReduction)
	for (int i = 1; i <= n; i++)
	{
		SumReduction += GetValue(i);
	}

	std::cout << SumReduction * 4 << std::endl;
}

void CountTimeOperation(int param, void (*func)(int))
{
	const auto start_time = std::chrono::steady_clock::now();
	func(param);
	const auto duration = std::chrono::steady_clock::now() - start_time;
	std::cout << std::chrono::duration<double>(duration).count() << " seconds" << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int n = pow(10, 7);
	CountTimeOperation(n, GetAndPrintPiResult);
	std::cout << std::endl;

	CountTimeOperation(n, GetAndPrintPiResultParallel);
	std::cout << std::endl;

	CountTimeOperation(n, GetAndPrintPiResultAtomic);
	std::cout << std::endl;

	CountTimeOperation(n, GetAndPrintPiResultReduction);
	std::cout << std::endl;

	return 0;
}
