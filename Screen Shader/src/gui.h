#pragma once
#include <windows.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "assets/icons_font_awesome_6.h"
#include <glad/glad.h>

class GUI{
public:
    bool Init(HWND hwnd);
    void Render(HWND hwnd);
    void Close();

private:
    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;  
};