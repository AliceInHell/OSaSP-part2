#include "Header.h"
#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>

using namespace std;

int Sum(int a, int b)
{
	return a + b;
}

int Sub(int a, int b)
{
	return a - b;
}

void SayHello()
{
	cout << "\nHELLO FROM INJECTION DLL, MF!\n" << endl;
}

void Replace(LPCSTR target, LPCSTR newStr)
{
	std::string targetString(target), newString(newStr);
	MEMORY_BASIC_INFORMATION mbi;
	SYSTEM_INFO si;
	CHAR page[4096];
	GetSystemInfo(&si);

	DWORD_PTR dwMemoryStart = (DWORD_PTR)si.lpMinimumApplicationAddress;
	DWORD_PTR dwMemoryEnd = (DWORD_PTR)si.lpMaximumApplicationAddress;

	HANDLE hProcess = GetCurrentProcess();

	for (DWORD_PTR addr = dwMemoryStart; addr < dwMemoryEnd; addr += si.dwPageSize)
	{
		VirtualQueryEx(hProcess, (LPCVOID)(addr), &mbi, sizeof(mbi));

		if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS && mbi.Type == MEM_IMAGE)
		{
			ZeroMemory(page, sizeof(page));
			ReadProcessMemory(hProcess, (LPCVOID)addr, page, sizeof(page), NULL);

			CHAR *match = begin(page);
			BOOL isMatch = false;
			while (match < end(page))
			{
				match = search(begin(page), end(page), targetString.begin(), targetString.end());
				if (match < end(page))
				{
					isMatch = true;
					copy(newString.begin(), newString.end(), match);
				}
			}

			if (isMatch)
			{
				WriteProcessMemory(hProcess, (LPVOID)addr, (LPCVOID)page, sizeof(page), NULL);
			}
		}
	}
}