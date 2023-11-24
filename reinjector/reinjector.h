#pragma once
#include "resource.h"
#include <Windows.h>
#include <stdio.h>
#include <Commdlg.h>
#include <CommCtrl.h>
#include <Psapi.h>
#include <tlhelp32.h>

// Global main window variables.
HWND hEdit1, hEdit2, hEdit3, hButton1, hButton2, hButton3, hButton4, hButton5, hCheckbox1, hCheckbox2, hCheckbox3;
HWND g_hwnd;
DWORD InjectionType = 0;
BOOL StartInject = FALSE;

// Global Shellcode Editor Variables
HWND hEditMultiline;
BYTE g_Shellcode[0x10000];
wchar_t g_TextContent[0x10000];

//Process List Variables
DWORD selected_process_id = 0;
WCHAR process_id_string[32];
DWORD g_processIds[4096];