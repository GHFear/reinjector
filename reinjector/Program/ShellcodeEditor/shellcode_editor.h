#pragma once
#include "../../reinjector.h"

LRESULT CALLBACK ShellCodeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
            SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
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
        SetWindowTextW(hEditMultiline, g_TextContent.c_str());
        SetWindowSubclass(hEditMultiline, EditSubclassProc, 0, 0);
        break;
    }
    case WM_SIZE:
        MoveWindow(hEditMultiline, 10, 10, LOWORD(lParam) - 30, HIWORD(lParam) - 20, TRUE);
        break;
    case WM_CLOSE:
    {
        int textLength = GetWindowTextLengthW(hEditMultiline);
        if (textLength % 2 == 0) {
            DestroyWindow(hwnd);
        }
        else {
            MessageBox(nullptr, L"Shellcode is NOT valid!", L"Shellcode Status", MB_OK);
        }
        break;
    }
    case WM_DESTROY:
    {
        int textLength = GetWindowTextLengthW(hEditMultiline);
        std::vector<wchar_t> buffer(textLength + 1);
        GetWindowTextW(hEditMultiline, buffer.data(), textLength + 1);
        g_TextContent = buffer.data();
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}