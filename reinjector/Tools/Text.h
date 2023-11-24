#pragma once
#include <Windows.h>

extern const char* WideStringToMultiByte(const WCHAR* wideStr)
{
    UINT char_count = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    if (char_count == 0) {
        return NULL;
    }

    char* multi_byte_str = malloc(char_count);
    if (WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, multi_byte_str, char_count, NULL, NULL) == 0) {
        free(multi_byte_str);
        return NULL;
    }
    return multi_byte_str;
}

extern BOOL HexStringToByteArray(const WCHAR* hex_string, BYTE* byte_array, INT array_size)
{
    INT hex_length = wcslen(hex_string);

    if (hex_length % 2 != 0 || hex_length / 2 > array_size)
    {
        return FALSE;
    }

    for (INT i = 0; i < hex_length; i += 2)
    {
        INT byte_value;
        swscanf_s(hex_string + i, L"%2x", &byte_value);

        if (byte_value < 0 || byte_value > 255) {
            return FALSE;
        }

        byte_array[i / 2] = (BYTE)byte_value;
    }

    return TRUE;
}