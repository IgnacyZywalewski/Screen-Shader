#include "gui.h"
#include "renderer.h"
#include <glad/glad.h>
#include "imgui/IconsFontAwesome6.h"

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

    //czcionka
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    float baseFontSize = 13.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, iconFontSize, &icons_config, icons_ranges);
}

void GUI::Render(HWND hwnd, ShadersData& shadersData)
{
    wglMakeCurrent(HDCGUI, GLContextGUI);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(350, 500));

    ImGui::Begin("Screen Shader", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );
    {
        ImGui::Text("Screen Shader");

        ImGui::SameLine(ImGui::GetWindowWidth() - 55);

        if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE))
            ShowWindow(hwnd, SW_MINIMIZE);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK)) PostQuitMessage(0);

        ImGui::Separator();
    }

    float labelWidth = 90.0f;
    float resetButtonWidth = 30.0f;
    float sliderWidth = ImGui::GetWindowWidth() - labelWidth - resetButtonWidth - 20.0f;

    //jasnosc
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Brightness");
    ImGui::SameLine(labelWidth);

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##brightness_slider", &shadersData.brightness, 0.2f, 3.0f);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_brightness", ImVec2(resetButtonWidth, 0)))
        shadersData.brightness = 1.0f;

    // gamma
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Gamma");
    ImGui::SameLine(labelWidth);

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##gamma_slider", &shadersData.gamma, 0.0f, 5.0f);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_gamma", ImVec2(resetButtonWidth, 0)))
        shadersData.gamma = 1.0f;

    // contrast
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Contrast");
    ImGui::SameLine(labelWidth);

    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##contrast_slider", &shadersData.contrast, -255.0f, 255.0f);
    ImGui::PopItemWidth();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_contrast", ImVec2(resetButtonWidth, 0)))
        shadersData.contrast = 0.0f;
    ImGui::NewLine();

    //inwersja kolorow
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Invert colors");
    ImGui::SameLine();
    ImGui::Checkbox("##color_inversion_checkbox", &shadersData.colorInversion);
    ImGui::NewLine();

    //kolory rgb
    ImGui::Text("Red");
    ImGui::SameLine();
    ImGui::Checkbox("##red_color_checkbox", &shadersData.redColor);
    ImGui::SameLine();

    ImGui::Text("Green");
    ImGui::SameLine();
    ImGui::Checkbox("##green_color_checkbox", &shadersData.greenColor);
    ImGui::SameLine();

    ImGui::Text("Blue");
    ImGui::SameLine();
    ImGui::Checkbox("##blue_color_checkbox", &shadersData.blueColor);

    //przeciaganie
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
