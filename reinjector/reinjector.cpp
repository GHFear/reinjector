// RE::LLUSORY DLL INJECTOR 0.2.1 | A Dll Injector By GHFear.
// Uses C/C++ and Win32 API.

#include "framework.h"
#include "reinjector.h"
#include "Injectors/CreateRemoteLoadLibrary/CreateRemoteLoadLibrary.h"
#include "Injectors/ReflectiveInjection/reflective_injection.h"
#include "Tools/Text.h"
#include "Program/ListProcesses/list_processes.h"
#include "Program/DllDialog/dll_dialog.h"
#include "Program/ShellcodeEditor/shellcode_editor.h"
#include "Injectors/LaunchAndInject/launch_and_inject.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("MyWindowClass");
    wc.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, _T("RE::LLUSORY DLL INJECTOR 0.2.1"),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
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

INT set_injection_type(DWORD selected_process_id,  const wchar_t* wide_dll_file_path, const wchar_t* wide_exe_file_path)
{
    if (InjectionType == 0)
    {
        const char* multi_bytes_dll_file_path = WideStringToMultiByte(wide_dll_file_path);
        if (multi_bytes_dll_file_path != NULL)
        {
            if (wide_exe_file_path != L"")
            {
                InjectDLL_WriteProcessMemory(selected_process_id, multi_bytes_dll_file_path);
            }
        }
        else
        {
            std::cerr << "Error converting wide string to multi-byte string." << std::endl;
            MessageBox(NULL, L"Error converting wide dll path string to multi-byte string.", L"Injection Status", MB_OK);
        }
        delete[] multi_bytes_dll_file_path;
    }
    else if (InjectionType == 1)
    {
        UINT textLength = sizeof(g_TextContent);

        if (HexStringToByteArray(g_TextContent, g_Shellcode, sizeof(g_Shellcode)))
        {
            InjectDll_Reflective(selected_process_id, g_Shellcode, sizeof(g_Shellcode));
        }
        else
        {
            std::cerr << "Error converting shellcode text to shellcode byte array." << std::endl;
            MessageBox(NULL, L"Error converting shellcode text to shellcode byte array.", L"Injection Status", MB_OK);
        }
    }
    else if (InjectionType == 2)
    {
        const char* multi_bytes_dll_file_path = WideStringToMultiByte(wide_dll_file_path);
        if (multi_bytes_dll_file_path != NULL)
        {
            if (wide_exe_file_path != L"" && StartInject == true)
            {
                LaunchAndInject(wide_exe_file_path, wide_dll_file_path);
            }
        }
        else
        {
            std::cerr << "Error converting wide string to multi-byte string." << std::endl;
            MessageBox(NULL, L"Error converting wide dll path string to multi-byte string.", L"Injection Status", MB_OK);
        }
        delete[] multi_bytes_dll_file_path;
    }

    return 0;
}

VOID checkbox_injection_type(WPARAM wParam, HWND checkbox, HWND button)
{
    for (UINT i = 105; i <= 107; ++i)
    {
        if (i != LOWORD(wParam))
        {
            SendMessage(GetDlgItem(checkbox, i), BM_SETCHECK, BST_UNCHECKED, 0);
        }
    }
    BOOL isChecked = IsDlgButtonChecked(checkbox, LOWORD(wParam));
    if (isChecked == true && LOWORD(wParam) == 106)
    {
        InjectionType = 0;
        StartInject = false;
        EnableWindow(hButton1, TRUE);
        EnableWindow(hButton2, TRUE);
        EnableWindow(hButton3, TRUE);
        EnableWindow(hButton4, FALSE);
        EnableWindow(hButton5, FALSE);
    }
    else if (isChecked == true && LOWORD(wParam) == 105)
    {
        InjectionType = 1;
        StartInject = false;
        EnableWindow(hButton1, TRUE);
        EnableWindow(hButton2, FALSE);
        EnableWindow(hButton3, TRUE);
        EnableWindow(hButton4, TRUE);
        EnableWindow(hButton5, FALSE);

    }
    else if (isChecked == true && LOWORD(wParam) == 107)
    {
        InjectionType = 2;
        StartInject = true;
        EnableWindow(hButton1, FALSE);
        EnableWindow(hButton2, TRUE);
        EnableWindow(hButton3, TRUE);
        EnableWindow(hButton4, FALSE);
        EnableWindow(hButton5, TRUE);
    }
    else
    {
        StartInject = false;
        EnableWindow(hButton1, FALSE);
        EnableWindow(hButton2, FALSE);
        EnableWindow(hButton3, FALSE);
        EnableWindow(hButton4, FALSE);
        EnableWindow(hButton5, FALSE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR wide_dll_file_path[MAX_PATH] = L"";
    static WCHAR wide_exe_file_path[MAX_PATH] = L"";
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

        hEdit3 = CreateWindowEx(0, _T("EDIT"), _T(""),
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
            10, 40, 150, 20,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton1 = CreateWindowEx(0, _T("BUTTON"), _T("Process"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 10, 100, 30,
            hwnd, (HMENU)1001, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton2 = CreateWindowEx(0, _T("BUTTON"), _T("Dll Path"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 50, 100, 30,
            hwnd, (HMENU)1002, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton3 = CreateWindowEx(0, _T("BUTTON"), _T("Inject"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 400, 100,
            hwnd, (HMENU)1003, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton4 = CreateWindowEx(0, _T("BUTTON"), _T("Edit Shellcode"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 90, 150, 30,
            hwnd, (HMENU)1004, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButton5 = CreateWindowEx(0, _T("BUTTON"), _T("Exe Path"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            240, 50, 100, 30,
            hwnd, (HMENU)1005, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hCheckbox1 = CreateWindowEx(0, _T("BUTTON"), _T("Shellcode"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 80, 100, 30,
            hwnd, (HMENU)105, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hCheckbox2 = CreateWindowEx(0, _T("BUTTON"), _T("DLL"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 80, 100, 30,
            hwnd, (HMENU)106, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hCheckbox3 = CreateWindowEx(0, _T("BUTTON"), _T("Start+Inject"),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            10, 80, 100, 30,
            hwnd, (HMENU)107, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        // To check the checkbox by default
        SendMessage(hCheckbox2, BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(hButton2, TRUE);
        EnableWindow(hButton3, TRUE);
        EnableWindow(hButton4, FALSE);
        EnableWindow(hButton5, FALSE);
        EnableWindow(hEdit1, FALSE);
        EnableWindow(hEdit2, FALSE);
        EnableWindow(hEdit3, FALSE);

        break;
    }
    case WM_SIZE:
    {
        MoveWindow(hEdit1, 10, 10, LOWORD(lParam) - 130, 30, TRUE);
        MoveWindow(hEdit2, 10, 50, LOWORD(lParam) - 130, 30, TRUE);
        MoveWindow(hEdit3, 10, 90, LOWORD(lParam) - 130, 30, TRUE);
        MoveWindow(hButton1, LOWORD(lParam) - 110, 10, 100, 30, TRUE);
        MoveWindow(hButton2, LOWORD(lParam) - 110, 50, 100, 30, TRUE);
        MoveWindow(hButton3, 10, HIWORD(lParam) - 40, 100, 30, TRUE);
        MoveWindow(hButton4, 120, HIWORD(lParam) - 40, 100, 30, TRUE);
        MoveWindow(hButton5, LOWORD(lParam) - 110, 90, 100, 30, TRUE);
        MoveWindow(hCheckbox1, LOWORD(lParam) - 110, HIWORD(lParam) - 80, 100, 30, TRUE);
        MoveWindow(hCheckbox2, LOWORD(lParam) - 110, HIWORD(lParam) - 40, 100, 30, TRUE);
        MoveWindow(hCheckbox3, LOWORD(lParam) - 110, HIWORD(lParam) - 120, 100, 30, TRUE);
        break;
    }
    case WM_CTLCOLORSTATIC:
    {
        if ((HWND)lParam == hCheckbox3 || (HWND)lParam == hCheckbox2 || (HWND)lParam == hCheckbox1)
        {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(255, 255, 255));
            SetBkColor(hdc, RGB(64, 64, 64));
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case 1001:
            if (create_process_list())
            {

            }
            break;
        case 1002:
            if (OpenFileDlg(hwnd, wide_dll_file_path, MAX_PATH, L"Dll Files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0"))
            {
                SetWindowTextW(hEdit2, wide_dll_file_path);
            }
            break;
        case 1003:
        {
            set_injection_type(selected_process_id, wide_dll_file_path, wide_exe_file_path);
            break;
        }   
        case 1004:
        {
            WNDCLASS textWindowClass = {};
            textWindowClass.lpfnWndProc = ShellCodeWindowProc;
            textWindowClass.hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            textWindowClass.lpszClassName = _T("MyTextWindowClass");
            textWindowClass.hbrBackground = CreateSolidBrush(RGB(64, 64, 64));
            RegisterClass(&textWindowClass);

            CreateWindowEx(0, textWindowClass.lpszClassName, _T("Shellcode Editor"),
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 400, 300,
                NULL, NULL, textWindowClass.hInstance, NULL);
            break;
        }
        case 1005:
        {
            if (OpenFileDlg(hwnd, wide_exe_file_path, MAX_PATH, L"Exe Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0"))
            {
                SetWindowTextW(hEdit3, wide_exe_file_path);
            }
            break;
        }
        case 105: 
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                checkbox_injection_type(wParam, hwnd, hButton1);
            }
            break;
        }
        case 106:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                checkbox_injection_type(wParam, hwnd, hButton2);
            }
            break;
        }
        case 107:
        {
            if (HIWORD(wParam) == BN_CLICKED)
            {
                checkbox_injection_type(wParam, hwnd, hButton5);
            }
            break;
        }
        }
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
        pMinMax->ptMinTrackSize.x = 500;
        pMinMax->ptMinTrackSize.y = 300;
        pMinMax->ptMaxTrackSize.x = 500;
        pMinMax->ptMaxTrackSize.y = 300;
        break;
    }
    case WM_DESTROY:
        DeleteObject(g_CheckboxBackgroundBrush);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}