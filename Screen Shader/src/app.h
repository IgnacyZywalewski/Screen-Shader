#pragma once
#include <Windows.h>
#include "renderer.h"
#include "gui.h"

class App {
public:
    App(HINSTANCE hInstance);
    int Run();

private:
    HINSTANCE hInstance;
    Renderer renderer;
    GUI gui;

    HWND hwndOverlay = nullptr;
    HWND hwndGUI = nullptr;

    static bool running;

    static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
