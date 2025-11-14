#pragma once
#include <Windows.h>
#include "renderer.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

class GUI{
public:
    bool Init(HWND hwnd, Renderer& renderer);
    void Render(HWND hwnd, ShadersData& shadersData);
    void Close();

private:
    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;
};
