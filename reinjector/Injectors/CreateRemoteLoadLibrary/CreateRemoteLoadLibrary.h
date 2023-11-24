#pragma once
#include "../../reinjector.h"

VOID InjectDLL_WriteProcessMemory(DWORD processId, const char* dllPath) {

    // Check if process id is 0
    if (processId == 0)
    {
        MessageBox(NULL, L"Process ID is null!", L"Injection Status", MB_OK);
        return;
    }

    // Check if shellcode is nullptr or 0 in size
    if (strlen(dllPath) == 0)
    {
        MessageBox(NULL, L"Dll Path is null!", L"Injection Status", MB_OK);
        return;
    }

    AllocConsole();
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        printf("Failed to open process. Error code: %d", GetLastError());
        return;
    }

    LPVOID remoteString = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (remoteString == NULL) {
        printf("Failed to allocate memory in the remote process. Error code: %d", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    if (!WriteProcessMemory(hProcess, remoteString, dllPath, strlen(dllPath) + 1, NULL)) {
        printf("Failed to write to remote process memory. Error code: %d", GetLastError());
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddr == NULL) {
        printf("Failed to get the address of LoadLibrary. Error code: %d", GetLastError());
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteString, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create remote thread. Error code: %d", GetLastError());
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    printf("DLL injected successfully!");
    MessageBox(NULL, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);
}