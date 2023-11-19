#pragma once
#include <Windows.h>
#include <sstream>
#include <iomanip>

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

bool HexStringToByteArray(const std::wstring& hexString, BYTE* byteArray, size_t arraySize)
{
    if (hexString.size() % 2 != 0 || hexString.size() / 2 > arraySize)
    {
        return false;
    }

    for (size_t i = 0; i < hexString.size(); i += 2)
    {
        std::wistringstream ss(hexString.substr(i, 2));
        int byteValue;
        ss >> std::hex >> byteValue;

        if (ss.fail()) {
            return false;  // Failed to convert a substring to an integer
        }

        byteArray[i / 2] = static_cast<BYTE>(byteValue);
    }

    return true;
}