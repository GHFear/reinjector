#pragma once
#include "../../reinjector.h"

bool LaunchAndInject(const wchar_t* targetExecutable, const wchar_t* dllPath)
{
    // Step 1: Create the process
    STARTUPINFO startupInfo = {};
    PROCESS_INFORMATION processInfo = {};

    if (!CreateProcess(targetExecutable, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
    {
        std::cerr << "Failed to create the process. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Step 2: Get the Process ID (PID)
    DWORD processId = processInfo.dwProcessId;

    // Step 3: Inject the DLL using the CreateRemoteThread method
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        std::cerr << "Failed to open the process. Error: " << GetLastError() << std::endl;
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return false;
    }

    LPVOID pRemoteCode = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (pRemoteCode == NULL)
    {
        std::cerr << "Failed to get the address of LoadLibraryW. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return false;
    }

    LPVOID pAllocatedMem = VirtualAllocEx(hProcess, NULL, wcslen(dllPath) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pAllocatedMem == NULL)
    {
        std::cerr << "Failed to allocate memory in the remote process. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pAllocatedMem, dllPath, wcslen(dllPath) * sizeof(wchar_t), NULL))
    {
        std::cerr << "Failed to write to the remote process memory. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return false;
    }

    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteCode, pAllocatedMem, 0, NULL);
    if (hRemoteThread == NULL)
    {
        std::cerr << "Failed to create a remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return false;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Clean up
    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return true;
}
