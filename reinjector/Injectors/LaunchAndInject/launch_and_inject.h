#pragma once
#include "../../reinjector.h"

BOOL ResumeSuspendedProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        return FALSE;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
            if (hThread != NULL) {
                ResumeThread(hThread);
                CloseHandle(hThread);
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);
    CloseHandle(hProcess);
    return TRUE;
}

bool LaunchAndInject(const wchar_t* targetExecutable, const wchar_t* dllPath)
{
    STARTUPINFO startupInfo = {};
    PROCESS_INFORMATION processInfo = {};

    if (!CreateProcess(targetExecutable, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
    {
        std::cerr << "Failed to create the process. Error: " << GetLastError() << std::endl;
        return false;
    }

    DWORD processId = processInfo.dwProcessId;

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

    WaitForSingleObject(hRemoteThread, INFINITE);

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    if (!ResumeSuspendedProcess(processId))
    {
        std::cerr << "Failed to resume the process. Error: " << GetLastError() << std::endl;
        return false;
    }

    std::cout << "DLL injected successfully!" << std::endl;
    MessageBox(NULL, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);

    return true;
}
