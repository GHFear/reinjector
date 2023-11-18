// RE::LLUSORY DLL INJECTOR 0.1 | A Dll Injector By GHFear.
// Uses C/C++ and Win32 API.

#include "framework.h"
#include "reinjector.h"
#include <Windows.h>
#include <iostream>
#include <Commdlg.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <Psapi.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND hEdit1, hEdit2, hButton1, hButton2, hButton3;
DWORD selected_process_id = 0;
WCHAR process_id_string[11];
DWORD g_processIds[1024] = {};
HWND g_hwnd;

const char* WideStringToMultiByte(const WCHAR* wideStr)
{
    int char_count = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (char_count == 0) {
        return nullptr;
    }

    char* multi_byte_str = new char[char_count];
    if (WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, multi_byte_str, char_count, NULL, NULL) == 0) {
        delete[] multi_byte_str;
        return nullptr;
    }
    return multi_byte_str;
}

void ListProcesses(HWND hwnd)
{
    HWND listBox = CreateWindowEx(0, _T("LISTBOX"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
        10, 10, 300, 200, hwnd, (HMENU)IDC_PROCESS_LIST, GetModuleHandle(NULL), NULL);

    int process_amount = 0;
    DWORD process_ids[1024] = {};
    DWORD bytes_returned = 0;

    if (listBox)
    {
        if (EnumProcesses(process_ids, sizeof(process_ids), &bytes_returned))
        {
            DWORD number_of_processes = bytes_returned / sizeof(DWORD);
            for (DWORD i = 0; i < number_of_processes; ++i)
            {
                DWORD processId = process_ids[i];
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                if (hProcess)
                {
                    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
                    g_processIds[process_amount] = processId;
                    HMODULE hModule;
                    DWORD cbNeeded;
                    if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
                    {
                        GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                    }
                    SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM)szProcessName);
                    CloseHandle(hProcess);
                    process_amount++;
                }
            }
        }
    }
}

LRESULT CALLBACK ProcessListWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        ListProcesses(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PROCESS_LIST:
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                HWND listBox = GetDlgItem(hwnd, IDC_PROCESS_LIST);
                int selected_index = SendMessage(listBox, LB_GETCURSEL, 0, 0);

                if (selected_index != LB_ERR)
                {
                    DWORD process_id = g_processIds[selected_index];
                    selected_process_id = process_id;
                    swprintf_s(process_id_string, L"%u", process_id);
                    SetWindowTextW(hEdit1, process_id_string);
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    return 0;
                }
            }
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int create_process_list()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = ProcessListWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = _T("ProcessListWindowClass");
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, _T("Process List"),
        WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX) | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, wc.hInstance, NULL);

    if (hwnd)
    {
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

void InjectDLL(DWORD processId, const char* dllPath) {
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
    MessageBox(nullptr, L"Injection Successful!", L"Injection Status", MB_OK);
}

bool OpenFileDlg(HWND hwnd, LPWSTR filePath, DWORD filePathSize)
{
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = filePathSize;
    ofn.lpstrFilter = L"Dll Files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = L"Select a File";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileName(&ofn);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("MyWindowClass");
    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, _T("RE::LLUSORY DLL INJECTOR"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 160,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, _T("Window creation failed!"), _T("Error"), MB_ICONERROR);
        return 1;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DeleteObject(wc.hbrBackground);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR wide_file_path[MAX_PATH] = L"";
    switch (uMsg)
    {
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);

        HBRUSH hBrush = CreateSolidBrush(RGB(64, 64, 64));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        return 1;  
    }
    case WM_CREATE:
    {
        hEdit1 = CreateWindowEx(0, _T("EDIT"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
            10, 10, 150, 20,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hEdit2 = CreateWindowEx(0, _T("EDIT"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
            10, 40, 150, 20,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton1 = CreateWindowEx(0, _T("BUTTON"), _T("Process"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 10, 100, 30,
            hwnd, (HMENU)101, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton2 = CreateWindowEx(0, _T("BUTTON"), _T("Dll"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 50, 100, 30,
            hwnd, (HMENU)102, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton3 = CreateWindowEx(0, _T("BUTTON"), _T("Inject"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 400, 100,
            hwnd, (HMENU)103, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        break;
    }
    case WM_SIZE:
        MoveWindow(hEdit1, 10, 10, LOWORD(lParam) - 130, 30, TRUE);
        MoveWindow(hEdit2, 10, 50, LOWORD(lParam) - 130, 30, TRUE);
        MoveWindow(hButton1, LOWORD(lParam) - 110, 10, 100, 30, TRUE);
        MoveWindow(hButton2, LOWORD(lParam) - 110, 50, 100, 30, TRUE);
        MoveWindow(hButton3, 10, HIWORD(lParam) - 70, LOWORD(lParam) - 20, 60, TRUE);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 101:
            if (create_process_list())
            {

            }
            break;
        case 102:
            if (OpenFileDlg(hwnd, wide_file_path, MAX_PATH))
            {
                SetWindowTextW(hEdit2, wide_file_path);
            }
            break;
        case 103:
            const char* multi_bytes_file_path = WideStringToMultiByte(wide_file_path);
            if (multi_bytes_file_path != nullptr)
            {
                InjectDLL(selected_process_id, multi_bytes_file_path);
                delete[] multi_bytes_file_path;
            }
            else 
            {
                std::cerr << "Error converting wide string to multi-byte string." << std::endl;
            }
            break;
        }
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
        pMinMax->ptMinTrackSize.x = 400;
        pMinMax->ptMinTrackSize.y = 200;
        pMinMax->ptMaxTrackSize.x = 400;
        pMinMax->ptMaxTrackSize.y = 200;
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}