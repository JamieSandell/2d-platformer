//
// Created by jamie.sandell on 19/10/2026.
//

#include <windows.h>

static LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_CLOSE:
            DestroyWindow(window_handle);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(window_handle, message, w_param, l_param);
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int command_show) {
    UNREFERENCED_PARAMETER(previous_instance);
    UNREFERENCED_PARAMETER(command_line);

    WNDCLASSEX window_class = {0};
    window_class.cbSize = sizeof(WNDCLASSEX);
    window_class.lpfnWndProc = window_procedure;
    window_class.hInstance = instance;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.lpszClassName = "WindowClass";

    if (!RegisterClassEx(&window_class)) {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HWND window_handle = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        window_class.lpszClassName,
        "2D Platformer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, // x, y
        CW_USEDEFAULT, CW_USEDEFAULT, // width, height
        NULL,
        NULL,
        instance,
        NULL);

    if (window_handle == NULL) {
        MessageBox(NULL, "Window Creation Failed", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(window_handle, command_show);
    UpdateWindow(window_handle);

    MSG message;

    while (GetMessage(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    return (int)message.wParam;
}