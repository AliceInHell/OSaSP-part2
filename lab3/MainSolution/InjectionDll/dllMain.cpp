#include <Windows.h>
#include <stdio.h>
#include <iostream>

#include "..\\SomeDll\Header.h"
#pragma comment(lib, "..\\Debug\\SomeDll.lib")

using namespace std;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SayHello();
		Replace("working fine...", "control lost");
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}