/*
* windows.cpp
*
* This file contains the declarations necessary for the program to compile on a Windows PC
*
*/

#pragma warning(disable:4996)

#include "stdafx.h"

// Required for Windows compiles only
#if IBM

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}

#endif