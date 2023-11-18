#pragma once

#include <Windows.h>
#include <iostream>

int InjectDll_Reflective(DWORD processId, BYTE shellcode[], uint32_t shellcode_size) {
    
    AllocConsole();
    if (processId == 0) {
        std::cerr << "Target process not found." << std::endl;
        return 1;
    }

    // Open the target process with the necessary permissions
    HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, processId);

    if (hProcess == NULL) {
        std::cerr << "Failed to open the target process: " << GetLastError() << std::endl;
        return 1;
    }

    // Allocate memory in the target process for the shellcode
    LPVOID remoteShellcode = VirtualAllocEx(hProcess, NULL, sizeof(shellcode), MEM_COMMIT, PAGE_READWRITE);

    if (remoteShellcode == NULL) {
        std::cerr << "Failed to allocate memory in the target process: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return 1;
    }

    // Write the shellcode to the allocated memory in the target process
    if (!WriteProcessMemory(hProcess, remoteShellcode, shellcode, sizeof(shellcode), NULL)) {
        std::cerr << "Failed to write to the target process: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Create a remote thread in the target process to execute the shellcode
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteShellcode, NULL, 0, NULL);
    if (hThread == NULL) {
        std::cerr << "Failed to create a remote thread: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 1;
    }

    // Wait for the remote thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up resources
    VirtualFreeEx(hProcess, remoteShellcode, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "DLL injected successfully!" << std::endl;
    MessageBox(nullptr, L"WriteProcessMemory / CreateRemoteThread Injection Successful!", L"Injection Status", MB_OK);

    return 0;
}