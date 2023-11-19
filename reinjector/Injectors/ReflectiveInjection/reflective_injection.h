#pragma once

#include <Windows.h>
#include <iostream>

int InjectDll_Reflective(DWORD processId, BYTE shellcode[], uint32_t shellcode_size) {
    
    if (processId == 0) {
        MessageBox(nullptr, L"Process ID is null!", L"Injection Status", MB_OK);
        return 1;
    }

    if (shellcode == nullptr || shellcode_size == 0)
    {
        MessageBox(nullptr, L"Shellcode is null!", L"Injection Status", MB_OK);
        return 1;
    }

    AllocConsole();
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);

    if (hProcess == NULL) {
        std::cerr << "Failed to open the target process: " << GetLastError() << std::endl;
        return 1;
    }

    LPVOID remoteShellcode = VirtualAllocEx(hProcess, NULL, shellcode_size, MEM_COMMIT, PAGE_READWRITE);

    if (remoteShellcode == NULL) {
        std::cerr << "Failed to allocate memory in the target process: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    if (!WriteProcessMemory(hProcess, remoteShellcode, shellcode, shellcode_size, NULL)) {
        std::cerr << "Failed to write to the target process: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteShellcode, NULL, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Failed to create a remote thread: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "DLL injected successfully!" << std::endl;
    MessageBox(nullptr, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);

    return 0;
}