#pragma once
#include "resource.h"
#include <Windows.h>
#include <iostream>
#include <Commdlg.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <vector>

HWND hEdit1, hEdit2, hEdit3, hButton1, hButton2, hButton3, hButton4, hButton5, hCheckbox1, hCheckbox2, hCheckbox3;
HWND g_hwnd;
DWORD InjectionType = 0;
BOOL StartInject = false;
BYTE g_Shellcode[0x4200] = {};
HWND hEditMultiline;
std::wstring g_TextContent;
WNDPROC DefaultEditProc;  // Declare a global variable to store the default window procedure