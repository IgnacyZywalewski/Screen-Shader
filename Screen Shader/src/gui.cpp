#include "gui.h"
#include "renderer.h"
#include <glad/glad.h>


void GUI::Init(HWND hwnd, Renderer& renderer)
{
    if (!renderer.InitOpenGL(hwnd, HDCGUI, GLContextGUI)){
        MessageBox(hwnd, L"Init GUI OpenGL failed", L"Error", MB_OK);
        return;
    }

    wglMakeCurrent(HDCGUI, GLContextGUI);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void GUI::Render(HWND hwnd, float& brightness, float& contrast)
{
    wglMakeCurrent(HDCGUI, GLContextGUI);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(300, 500));
    ImGui::Begin("Screen Shader", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    ImGui::Text("Screen Shader");

    ImGui::SameLine(250);
    if (ImGui::Button("-")) ShowWindow(hwnd, SW_MINIMIZE);
    ImGui::SameLine();
    if (ImGui::Button("X")) PostQuitMessage(0);

    ImGui::SliderFloat("Brightness", &brightness, -1.0f, 1.0f);
    ImGui::SliderFloat("Contrast", &contrast, -50.0f, 50.0f);

    static POINT dragOffset = { 0,0 };
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
        POINT p;
        GetCursorPos(&p);
        RECT r;
        GetWindowRect(hwnd, &r);
        dragOffset.x = p.x - r.left;
        dragOffset.y = p.y - r.top;
    }
    if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) {
        POINT p; GetCursorPos(&p);
        SetWindowPos(hwnd, nullptr, p.x - dragOffset.x, p.y - dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(HDCGUI);
}

void GUI::Close()
{
    wglMakeCurrent(HDCGUI, GLContextGUI);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(NULL, NULL);

    if (GLContextGUI) 
        wglDeleteContext(GLContextGUI);

    if (HDCGUI) 
        ReleaseDC(nullptr, HDCGUI);
}
