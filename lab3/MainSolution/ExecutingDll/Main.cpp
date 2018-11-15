#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <tlhelp32.h>

#include "..\\SomeDll\\Header.h"
#pragma comment(lib, "..\\Debug\\SomeDll.lib")

using namespace std;

typedef int Func(int, int);

const string anotherProcessName("DemoProcess.exe");

const int n = 5;
string strings[n] = {
	"first string",
	"second string",
	"third string",
	"fourth string",
	"fifth string"
};


DWORD GetProcessIdByProcessName(string processName)
{
	DWORD processId = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 processEntry;

	ZeroMemory(&processEntry, sizeof(processEntry));
	processEntry.dwSize = sizeof(processEntry);

	bool isFound = false;
	while (Process32Next(hSnapshot, &processEntry) && !isFound)
	{
		if (!processName.compare(processEntry.szExeFile))
		{
			processId = processEntry.th32ProcessID;
			isFound = true;
		}
	}

	return processId;
}

void main()
{
	//memory check
	cout << "Strings: \n" << endl;
	for (int i = 0; i < n; ++i)
		cout << strings[i].c_str() << endl;

	cout << "" << endl;
	cout << "Replacing..." << endl;
	cout << "" << endl;

	Replace(strings[2].c_str(), "new replacing string");

	cout << "Strings: " << endl;
	for (int i = 0; i < n; ++i)
		cout << strings[i].c_str() << endl;
	cout << "" << endl;


	//dll test's
	cout << "Using static includes: " << endl << "3 + 7 = " << Sum(3, 7) << endl << "3 - 7 = " << Sub(3, 7) << endl;

	HMODULE dll = 0;
	if ((dll = LoadLibrary("SomeDll"))) {
		Func *_Sum, *_Sub;
		_Sum = (Func*)GetProcAddress(dll, MAKEINTRESOURCE(4));
		_Sub = (Func*)GetProcAddress(dll, MAKEINTRESOURCE(3));
		cout << "Using dynamic includes: " << endl << "3 + 7 = " << _Sum(3, 7) << endl << "3 - 7 = " << _Sub(3, 7) << endl;
		FreeLibrary(dll);
	}
	else
		printf("Cant load Dll");


	//dll injection
	cout << "\nLets call dll function from another process\n" << endl;
	_getch();

	HANDLE hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetProcessIdByProcessName(anotherProcessName));

	if (hRemoteProcess != NULL)
	{
		cout << "Already got process id" << endl;

		LPVOID threadFunction = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");

		string argument("InjectionDll.dll");
		LPVOID argumentAddress = VirtualAllocEx(hRemoteProcess, NULL, argument.length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		WriteProcessMemory(hRemoteProcess, (LPVOID)argumentAddress, argument.c_str(), argument.length() + 1, NULL);

		if (CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)threadFunction, (LPVOID)argumentAddress, 0, NULL))
		{
			Sleep(1000);
			cout << "Created remote thread" << endl;
			CloseHandle(hRemoteProcess);
		}
		else
			cout << "Cant create remote thread" << endl;

	}
	else
		cout << "Cant get process id" << endl;

	Sleep(2000);
	cout << "\nEnter key for exit" << endl;
	_getch();
}