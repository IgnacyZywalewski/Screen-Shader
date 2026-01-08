#include <vector>
#include <cassert>

#include "app.h"
#include "screen_flips.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

App::App(HINSTANCE hInstance)
    : hInstance(hInstance) {
}

bool App::running = true;

LRESULT CALLBACK App::OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY: return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK App::GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        normalScreen();
        PostQuitMessage(0);
        running = false;
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void App::Run() {
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    const wchar_t OVERLAY_CLASS[] = L"OverlayWindowClass";
    const wchar_t GUI_CLASS[] = L"GUIWindowClass";

    // klasa nakladki
    WNDCLASS wcOverlay = {};
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = hInstance;
    wcOverlay.lpszClassName = OVERLAY_CLASS;
    RegisterClass(&wcOverlay);

    hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        OVERLAY_CLASS, L"Overlay", WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );

    SetLayeredWindowAttributes(hwndOverlay, 0, 255, LWA_ALPHA);
    ShowWindow(hwndOverlay, SW_SHOW);
    SetWindowDisplayAffinity(hwndOverlay, WDA_EXCLUDEFROMCAPTURE);


    //klasa gui
    WNDCLASS wcGUI = {};
    wcGUI.lpfnWndProc = GUIWndProc;
    wcGUI.hInstance = hInstance;
    wcGUI.lpszClassName = GUI_CLASS;
    RegisterClass(&wcGUI);

    hwndGUI = CreateWindowEx(
        WS_EX_TOPMOST,
        GUI_CLASS, L"Screen Shader", WS_POPUP,
        screenWidth / 2 + 300, screenHeight / 2 - 300, 350, 550,
        nullptr, nullptr, hInstance, nullptr
    );
    ShowWindow(hwndGUI, SW_SHOW);
    SetWindowDisplayAffinity(hwndGUI, WDA_EXCLUDEFROMCAPTURE);


    if (!renderer.Init(hwndOverlay, hwndGUI, screenWidth, screenHeight)) {
        MessageBox(hwndOverlay, L"Renderer init failed", L"Error", MB_OK);
        return;
    }

    if (!gui.Init(hwndGUI)) {
        MessageBox(hwndGUI, L"Init GUI OpenGL failed", L"Error", MB_OK);
        return;
    }

    //petla
    MSG msg = {};
    while (running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) running = false;
        }

        renderer.Update();
        renderer.RenderOverlay();
        gui.Render(hwndGUI);
    }

    gui.Close();
    renderer.Close(hwndOverlay, hwndGUI);

}