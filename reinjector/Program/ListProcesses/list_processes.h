#pragma once
#include "../../reinjector.h"

extern VOID ListProcesses(HWND hwnd)
{
    HWND list_box = CreateWindowEx(0, _T("LISTBOX"), NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
        10, 10, 360, 240, hwnd, (HMENU)IDC_PROCESS_LIST, GetModuleHandle(NULL), NULL);

    UINT process_amount = 0;
    DWORD process_ids[1024];
    DWORD bytes_returned = 0;

    if (list_box)
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
                    HMODULE hModule = { 0 };
                    DWORD cbNeeded = 0;
                    if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded))
                    {
                        GetModuleBaseName(hProcess, hModule, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
                    }
                    SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)szProcessName);
                    CloseHandle(hProcess);
                    process_amount++;
                }
            }
        }
    }
}

extern LRESULT CALLBACK ProcessListWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        ListProcesses(hwnd);
        HICON hIcon = (HICON)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
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
                HWND list_box = GetDlgItem(hwnd, IDC_PROCESS_LIST);
                INT selected_index = SendMessage(list_box, LB_GETCURSEL, 0, 0);

                if (selected_index != LB_ERR)
                {
                    DWORD process_id = g_processIds[selected_index];
                    selected_process_id = process_id;
                    swprintf_s(process_id_string, _countof(process_id_string), L"%u", selected_process_id);
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

extern BOOL CreateProcessList()
{
    WNDCLASS ProcessListWindowClass = { 0 };
    ProcessListWindowClass.lpfnWndProc = ProcessListWindowProc;
    ProcessListWindowClass.hInstance = GetModuleHandle(NULL);
    ProcessListWindowClass.lpszClassName = _T("ProcessListWindowClass");
    ProcessListWindowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&ProcessListWindowClass);

    HWND hwnd = CreateWindowEx(0, ProcessListWindowClass.lpszClassName, _T("Process List"),
        WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX) | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, ProcessListWindowClass.hInstance, NULL);

    if (hwnd)
    {
        MSG msg = { 0 };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return TRUE;
}