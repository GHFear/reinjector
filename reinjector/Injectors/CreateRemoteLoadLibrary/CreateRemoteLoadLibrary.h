#pragma once
#include <Windows.h>
#include <iostream>

void InjectDLL_WriteProcessMemory(DWORD processId, const char* dllPath) {
    AllocConsole();
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process. Error code: " << GetLastError() << std::endl;
        return;
    }

    LPVOID remoteString = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (remoteString == NULL) {
        std::cerr << "Failed to allocate memory in the remote process. Error code: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return;
    }

    if (!WriteProcessMemory(hProcess, remoteString, dllPath, strlen(dllPath) + 1, NULL)) {
        std::cerr << "Failed to write to remote process memory. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (loadLibraryAddr == NULL) {
        std::cerr << "Failed to get the address of LoadLibrary. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteString, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Failed to create remote thread. Error code: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, remoteString, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);
    std::cout << "DLL injected successfully!" << std::endl;
    MessageBox(nullptr, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);
}