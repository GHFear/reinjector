#pragma once

#include <Windows.h>

INT InjectDll_Reflective(DWORD processId, BYTE shellcode[], UINT shellcode_size) {
    
    if (processId == 0) {
        MessageBox(NULL, L"Process ID is null!", L"Injection Status", MB_OK);
        return 1;
    }

    if (shellcode == NULL || shellcode_size == 0)
    {
        MessageBox(NULL, L"Shellcode is null!", L"Injection Status", MB_OK);
        return 1;
    }

    AllocConsole();
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);

    if (hProcess == NULL) {
        printf("Failed to open the target process: %d", GetLastError());
        return 1;
    }

    LPVOID remoteShellcode = VirtualAllocEx(hProcess, NULL, shellcode_size, MEM_COMMIT, PAGE_READWRITE);

    if (remoteShellcode == NULL) {
        printf("Failed to allocate memory in the target process: %d", GetLastError());
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, remoteShellcode, shellcode, shellcode_size, NULL)) {
        printf("Failed to write to the target process: %d", GetLastError());
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteShellcode, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create a remote thread: %d", GetLastError());
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    printf("DLL injected successfully!");
    MessageBox(NULL, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);

    return 0;
}