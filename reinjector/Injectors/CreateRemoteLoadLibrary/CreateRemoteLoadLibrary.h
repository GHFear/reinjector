#pragma once
#include "../../reinjector.h"

void InjectDLL_WriteProcessMemory(DWORD processId, const char* dllPath) {

    // Check if process id is 0
    if (processId == 0)
    {
        MessageBox(nullptr, L"Process ID is null!", L"Injection Status", MB_OK);
        return;
    }

    // Check if shellcode is nullptr or 0 in size
    if (strlen(dllPath) == 0)
    {
        MessageBox(nullptr, L"Dll Path is null!", L"Injection Status", MB_OK);
        return;
    }

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