#include "gui.h"
#include "renderer.h"
#include <glad/glad.h>
#include "assets/icons_font_awesome_6.h"

bool GUI::Init(HWND hwnd, Renderer& renderer)
{
    if (!renderer.InitOpenGL(hwnd, HDCGUI, GLContextGUI)){
        return false;
    }

    wglMakeCurrent(HDCGUI, GLContextGUI);

    /*if (!wglShareLists(renderer.GLContextOverlay, GLContextGUI)) {
        MessageBox(hwnd, L"Nie udało się współdzielić kontekstów OpenGL!", L"Błąd", MB_OK);
    }*/

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    return true;
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
            POINT p; 
            GetCursorPos(&p);
            SetWindowPos(hwnd, nullptr, p.x - dragOffset.x, p.y - dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }


        ImGui::Text("Screen Shader");

        ImGui::SameLine(ImGui::GetWindowWidth() - 55);

        if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE))
            ShowWindow(hwnd, SW_MINIMIZE);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK)) PostQuitMessage(0);

        ImGui::Separator();
    }

    float labelWidth = 90.0f;
    float buttonWidth = 30.0f;
    float sliderWidth = ImGui::GetWindowWidth() - labelWidth - buttonWidth - 20.0f;

    //jasnosc
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Brightness");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##brightness_slider", &shadersData.brightness, 0.5f, 4.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_brightness", ImVec2(buttonWidth, 0)))
        shadersData.brightness = 1.0f;

    // gamma
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Gamma");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##gamma_slider", &shadersData.gamma, 0.5f, 4.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_gamma", ImVec2(buttonWidth, 0)))
        shadersData.gamma = 1.0f;

    // kontrast
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Contrast");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##contrast_slider", &shadersData.contrast, -50.0f, 50.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_contrast", ImVec2(buttonWidth, 0)))
        shadersData.contrast = 0.0f;

    // nasycenie
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Saturation");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderFloat("##saturation_slider", &shadersData.saturation, 0.0f, 3.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_saturation", ImVec2(buttonWidth, 0)))
        shadersData.saturation = 1.0f;
    ImGui::NewLine();

    //kolory rgb
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Red");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth - buttonWidth - 8.0f);
    ImGui::SliderFloat("##red_slider", &shadersData.red, 0.0f, 2.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_XMARK "##zero_red", ImVec2(buttonWidth, 0)))
        shadersData.red = 0.0f;
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_red", ImVec2(buttonWidth, 0)))
        shadersData.red = 1.0f;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Green");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth - buttonWidth - 8.0f);
    ImGui::SliderFloat("##green_slider", &shadersData.green, 0.0f, 2.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_XMARK "##zero_green", ImVec2(buttonWidth, 0)))
        shadersData.green = 0.0f;
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_green", ImVec2(buttonWidth, 0)))
        shadersData.green = 1.0f;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Blue");
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth - buttonWidth - 8.0f);
    ImGui::SliderFloat("##blue_slider", &shadersData.blue, 0.0f, 2.0f);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_XMARK "##zero_blue", ImVec2(buttonWidth, 0)))
        shadersData.blue = 0.0f;
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_blue", ImVec2(buttonWidth, 0)))
        shadersData.blue = 1.0f;

    //inwersja kolorow
    ImGui::NewLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Invert colors");
    ImGui::SameLine();
    ImGui::Checkbox("##color_inversion_checkbox", &shadersData.colorInversion);
    ImGui::NewLine();

    //filtr czarno-bialy
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Black-White filter");
    ImGui::SameLine();
    ImGui::Checkbox("##black_white_checkbox", &shadersData.blackWhite);
    ImGui::NewLine();

    //zamiana hotyzontalna
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Horizontal Swap");
    ImGui::SameLine();
    ImGui::Checkbox("##horizontal_swap_checkbox", &shadersData.horizontalSwap);

    //zamiana wertykalna
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Vertical Swap");
    ImGui::SameLine();
    ImGui::Checkbox("##vertical_swap_checkbox", &shadersData.verticalSwap);
    ImGui::NewLine();

    //blur
    ImGui::NewLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Blur");
    ImGui::SameLine();
    ImGui::Checkbox("##blur_checkbox", &shadersData.blur);
    ImGui::SameLine(labelWidth);
    ImGui::PushItemWidth(sliderWidth);
    ImGui::SliderInt("##blur_radius_slider", &shadersData.blurRadius, 1, 5);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##zero_blur", ImVec2(buttonWidth, 0)))
        shadersData.blurRadius = 1;

    //emboss
    ImGui::NewLine();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Emboss filter");
    ImGui::SameLine();
    ImGui::Checkbox("##emboss_checkbox", &shadersData.emboss);
    ImGui::NewLine();


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
