#pragma once
#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

class Renderer;

class GUI{
public:
    void Init(HWND hwnd, Renderer& renderer);
    void Render(HWND hwnd, float& brightness, float& contrast);
    void Close();

private:
    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;
};
