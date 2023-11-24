#pragma once
#include "../../reinjector.h"

BOOL ResumeSuspendedProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL) {
        return FALSE;
    }

    THREADENTRY32 te32 = { 0 };
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

BOOL LaunchAndInject(const wchar_t* targetExecutable, const wchar_t* dllPath)
{
    STARTUPINFO startupInfo = { 0 };
    PROCESS_INFORMATION processInfo = { 0 };

    if (!CreateProcess(targetExecutable, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo))
    {
        printf("Failed to create the process. Error: %d", GetLastError());
        return FALSE;
    }

    DWORD processId = processInfo.dwProcessId;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        printf("Failed to open the process. Error: %d", GetLastError());
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return FALSE;
    }

    LPVOID pRemoteCode = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (pRemoteCode == NULL)
    {
        printf("Failed to get the address of LoadLibraryW. Error: %d", GetLastError());
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return FALSE;
    }

    LPVOID pAllocatedMem = VirtualAllocEx(hProcess, NULL, wcslen(dllPath) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (pAllocatedMem == NULL)
    {
        printf("Failed to allocate memory in the remote process. Error: %d", GetLastError());
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pAllocatedMem, dllPath, wcslen(dllPath) * sizeof(wchar_t), NULL))
    {
        printf("Failed to write to the remote process memory. Error: %d", GetLastError());
        VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return FALSE;
    }

    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pRemoteCode, pAllocatedMem, 0, NULL);
    if (hRemoteThread == NULL)
    {
        printf("Failed to create a remote thread. Error: %d", GetLastError());
        VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        return FALSE;
    }

    WaitForSingleObject(hRemoteThread, INFINITE);

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, pAllocatedMem, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    if (!ResumeSuspendedProcess(processId))
    {
        printf("Failed to resume the process. Error: %d", GetLastError());
        return FALSE;
    }

    printf("DLL injected successfully!");
    MessageBox(NULL, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);

    return TRUE;
}
