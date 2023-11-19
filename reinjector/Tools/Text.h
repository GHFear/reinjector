#pragma once
#include <Windows.h>
#include <sstream>
#include <iomanip>

const char* WideStringToMultiByte(const WCHAR* wideStr)
{
    UINT char_count = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (char_count == 0) {
        return NULL;
    }

    char* multi_byte_str = new char[char_count];
    if (WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, multi_byte_str, char_count, NULL, NULL) == 0) {
        delete[] multi_byte_str;
        return NULL;
    }
    return multi_byte_str;
}

bool HexStringToByteArray(const std::wstring& hex_string, BYTE* byte_array, size_t array_size)
{
    if (hex_string.size() % 2 != 0 || hex_string.size() / 2 > array_size)
    {
        return false;
    }

    for (size_t i = 0; i < hex_string.size(); i += 2)
    {
        std::wistringstream string_stream(hex_string.substr(i, 2));
        INT byte_value;
        string_stream >> std::hex >> byte_value;

        if (string_stream.fail()) {
            return false;
        }

        byte_array[i / 2] = static_cast<BYTE>(byte_value);
    }

    return true;
}