#include <iostream>
#include <windows.h>
#include <string>

DWORD WINAPI ThreadProc(const LPVOID lpParam)
{
    printf("Поток №%d выполняет свою работу\n", *(int*)lpParam + 1);
    ExitThread(0);
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    int n = 0;
    if (argc > 1)
    {
        n = *(argv[1]) - '0';
    }
    else
    {
        return 1;
    }
    HANDLE* handles = new HANDLE[n];
    for (int i = 0; i < n; i++)
    {
        handles[i] = CreateThread(NULL, 0, &ThreadProc, (LPVOID)&i, CREATE_SUSPENDED, NULL);
        ResumeThread(handles[i]);
    }
   
    WaitForMultipleObjects(n, handles, true, INFINITE);
    return 0;
}