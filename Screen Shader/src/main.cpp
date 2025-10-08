#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

#include "glad/glad.h"

#include "application.h"

#include <Windows.h>


LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
)
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    const wchar_t OVERLAY_CLASS[] = L"OverlayWindowClass";
    const wchar_t GUI_CLASS[] = L"GUIWindowClass";

    WNDCLASS wcOverlay = {};
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = hInstance;
    wcOverlay.lpszClassName = OVERLAY_CLASS;
    wcOverlay.hbrBackground = nullptr;

    RegisterClass(&wcOverlay);

    WNDCLASS wcGUI = {};
    wcGUI.lpfnWndProc = GUIWndProc;
    wcGUI.hInstance = hInstance;
    wcGUI.lpszClassName = GUI_CLASS;
    wcGUI.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wcGUI);

    HWND hwndOverlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        OVERLAY_CLASS,
        L"Overlay",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    SetLayeredWindowAttributes(hwndOverlay, 0, 255, LWA_ALPHA);
    ShowWindow(hwndOverlay, SW_SHOW);

    
    HWND hwndGUI = CreateWindowEx(
        0,
        GUI_CLASS,
        L"Ustawienia Filtra",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 400,
        nullptr, nullptr, hInstance, nullptr
    );

    ShowWindow(hwndGUI, SW_SHOW);


    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}