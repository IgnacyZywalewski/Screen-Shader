#pragma once
#include <Windows.h>
#include "renderer.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

struct GUIData {
    float lastHeight = 0.0f;
    float titleBarHeight = 40.0f;
    float buttonWidth = 24.0f;
    float labelWidth = 90.0f;
    float offset = 10.0f;
    
    bool nightMode = true;
    bool collapsed = false;

    bool firstFrame = true;
    bool firstFrameCC = true;
    bool firstFrameFIL = true;
    bool firstFrameFL = true;
    bool firstFrameED = true;
    bool firstFrameB = true;

};

class GUI{
public:
    bool Init(HWND hwnd, Renderer& renderer);
    void Render(HWND hwnd, ShadersData& shadersData);
    void Close();

private:
    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;

    GUIData guiData;
};
