#include <windows.h>
#include <string>
#include <iostream>
#include "tchar.h"
#include <fstream>

CRITICAL_SECTION FileLockingCriticalSection;

HANDLE ghMutex;

int ReadFromFile() {
	std::fstream myfile("balance.txt", std::ios_base::in);
	int result;
	myfile >> result;
	myfile.close();

	return result;
}

void ClearFile()
{
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile.clear();
	myfile.close();
}

void WriteToFile(int data) {
	std::fstream myfile("balance.txt", std::ios_base::out);
	myfile << data << std::endl;
	myfile.close();
}

int GetBalance() {
	int balance = ReadFromFile();
	return balance;
}

void Deposit(int money) {
	int balance = GetBalance();
	balance += money;

	WriteToFile(balance);
	printf("Balance after deposit: %d\n", balance);
}

void Withdraw(int money) {
	if (GetBalance() < money) {
		printf("Cannot withdraw money, balance lower than %d\n", money);
		return;
	}

	Sleep(20);
	int balance = GetBalance();
	balance -= money;
	WriteToFile(balance);
	printf("Balance after withdraw: %d\n", balance);
}

DWORD WINAPI DoDeposit(CONST LPVOID lpParameter)
{
	EnterCriticalSection(&FileLockingCriticalSection);
	Deposit((int)lpParameter);
	LeaveCriticalSection(&FileLockingCriticalSection);
	ExitThread(0);
}

DWORD WINAPI DoWithdraw(CONST LPVOID lpParameter)
{
	EnterCriticalSection(&FileLockingCriticalSection);
	Withdraw((int)lpParameter);
	LeaveCriticalSection(&FileLockingCriticalSection);
	ExitThread(0);
}

DWORD WINAPI DoDepositWithMutex(CONST LPVOID lpParameter)
{
	DWORD dwWaitResult;

	// Request ownership of mutex.
	dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try {
			Deposit((int)lpParameter);
		}

		__finally {
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
				std::cout << "Got a error!" << std::endl;
			}
		}
		break;

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED:
		return FALSE;
	}
	ExitThread(0);
	return TRUE;
}

DWORD WINAPI DoWithdrawWithMutex(CONST LPVOID lpParameter)
{
	DWORD dwWaitResult;

	// Request ownership of mutex.

	dwWaitResult = WaitForSingleObject(
		ghMutex,    // handle to mutex
		INFINITE);  // no time-out interval

	switch (dwWaitResult)
	{
		// The thread got ownership of the mutex
	case WAIT_OBJECT_0:
		__try {
			Withdraw((int)lpParameter);
		}

		__finally {
			// Release ownership of the mutex object
			if (!ReleaseMutex(ghMutex))
			{
				// Handle error.
				std::cout << "Got a error!" << std::endl;
			}
		}
		break;

		// The thread got ownership of an abandoned mutex
		// The database is in an indeterminate state
	case WAIT_ABANDONED:
		return FALSE;
	}
	ExitThread(0);
	return TRUE;
}

void StartDepositAndWithdrawWithMutex(HANDLE* handles)
{
	for (int i = 0; i < 50; i++) {
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDepositWithMutex, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdrawWithMutex, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}
}

void StartDepositAndWithdraw(HANDLE* handles)
{
	for (int i = 0; i < 50; i++) {
		handles[i] = (i % 2 == 0)
			? CreateThread(NULL, 0, &DoDeposit, (LPVOID)230, CREATE_SUSPENDED, NULL)
			: CreateThread(NULL, 0, &DoWithdraw, (LPVOID)1000, CREATE_SUSPENDED, NULL);
		ResumeThread(handles[i]);
	}
}

void RunWithCriticalSection()
{
	HANDLE* handles = new HANDLE[50];

	InitializeCriticalSection(&FileLockingCriticalSection);
	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	StartDepositAndWithdraw(handles);


	// ожидание окончания работы 50 потоков
	WaitForMultipleObjects(50, handles, true, INFINITE);
	printf("Final Balance: %d\n", GetBalance());

	getchar();

	DeleteCriticalSection(&FileLockingCriticalSection);
}

void RunWithMutex()
{
	ClearFile();
	HANDLE* handlesForMutex = new HANDLE[50];

	ghMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	WriteToFile(0);

	SetProcessAffinityMask(GetCurrentProcess(), 1);
	StartDepositAndWithdrawWithMutex(handlesForMutex);

	// ожидание окончания работы 50 потоков
	WaitForMultipleObjects(50, handlesForMutex, true, INFINITE);
	CloseHandle(ghMutex);
	printf("Final Balance: %d\n", GetBalance());

	getchar();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//std::cout << "-------------CriticalSection-------------" << std::endl;
	//RunWithCriticalSection();
	std::cout << "-------------With mutex-------------" << std::endl;
	RunWithMutex();

	return 0;
}
