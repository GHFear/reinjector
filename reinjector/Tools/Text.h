#pragma once
#include <Windows.h>

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