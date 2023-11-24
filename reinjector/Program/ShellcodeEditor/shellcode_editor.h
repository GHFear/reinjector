#pragma once
#include "../../reinjector.h"

LRESULT CALLBACK ShellCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

VOID CreateShellcodeWindow(HWND* hwnd)
{
    WNDCLASS ShellcodeWindowClass = { 0 };
    ShellcodeWindowClass.lpfnWndProc = ShellCodeWindowProc;
    ShellcodeWindowClass.hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    ShellcodeWindowClass.lpszClassName = _T("ShellcodeWindowClass");
    ShellcodeWindowClass.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
    ShellcodeWindowClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

    RegisterClass(&ShellcodeWindowClass);

    CreateWindowEx(0, ShellcodeWindowClass.lpszClassName, _T("Shellcode Editor"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
        NULL, NULL, ShellcodeWindowClass.hInstance, NULL);
}

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_CHAR:
    {
        if ((wParam >= '0' && wParam <= '9') || (wParam >= 'A' && wParam <= 'F') || (wParam >= 'a' && wParam <= 'f') || wParam == VK_BACK) {
            LRESULT lRes = DefSubclassProc(hwnd, uMsg, wParam, lParam);
            return lRes;
        }
        else {
            return 0;
        }
    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN) {
            SendMessage(GetParent(hwnd), WM_CLOSE, NULL, NULL);
            return 0;
        }
        else {
            return DefSubclassProc(hwnd, uMsg, wParam, lParam);
        }
    }
    default:
    {
        LRESULT lRes = DefSubclassProc(hwnd, uMsg, wParam, lParam);
        return lRes;
    }   
    }
}

LRESULT CALLBACK ShellCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        hEditMultiline = CreateWindowEx(0, _T("EDIT"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 300, 200,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        SendMessage(hEditMultiline, EM_LIMITTEXT, (WPARAM)0x10000, 0);
        SetWindowTextW(hEditMultiline, g_TextContent);
        SetWindowSubclass(hEditMultiline, EditSubclassProc, 0, 0);
        break;
    }
    case WM_SIZE:
        MoveWindow(hEditMultiline, 10, 10, LOWORD(lParam) - 30, HIWORD(lParam) - 20, TRUE);
        break;
    case WM_CLOSE:
    {
        UINT textLength = GetWindowTextLengthW(hEditMultiline);
        if (textLength % 2 == 0) {
            DestroyWindow(hwnd);
        }
        else {
            MessageBox(NULL, L"Shellcode is NOT valid!", L"Shellcode Status", MB_OK);
        }
        break;
    }
    case WM_DESTROY:
    {
        UINT textLength = GetWindowTextLengthW(hEditMultiline);
        wchar_t* buffer = (wchar_t*)malloc((textLength + 1) * sizeof(wchar_t));
        GetWindowTextW(hEditMultiline, buffer, textLength + 1);
        g_TextContent[0x10000];
        wcscpy_s(g_TextContent, _countof(g_TextContent), buffer);
        free(buffer);
        
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}