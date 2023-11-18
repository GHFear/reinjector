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
#include <tlhelp32.h>
#include "Injectors/CreateRemoteLoadLibrary/CreateRemoteLoadLibrary.h"
#include "Injectors/ReflectiveInjection/reflective_injection.h"
#include "Tools/Text.h"
#include "Program/ListProcesses/list_processes.h"
#include "Program/DllDialog/dll_dialog.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
                InjectDLL_WriteProcessMemory(selected_process_id, multi_bytes_file_path);
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