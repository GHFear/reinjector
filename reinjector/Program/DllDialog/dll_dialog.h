#pragma once
#include <Windows.h>

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