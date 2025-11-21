#include "gui.h"
#include "renderer.h"
#include <glad/glad.h>
#include "assets/icons_font_awesome_6.h"

bool GUI::Init(HWND hwnd, Renderer& renderer) {
    if (!renderer.InitOpenGL(hwnd, HDCGUI, GLContextGUI)) {
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

void GUI::Render(HWND hwnd, ShadersData& shadersData) {
    wglMakeCurrent(HDCGUI, GLContextGUI);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (guiData.firstFrame) {
        ImGui::SetNextWindowSize(ImVec2(350, 550));
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(250, guiData.titleBarHeight), ImVec2(FLT_MAX, FLT_MAX));

  
    ImGui::Begin("Screen Shader", nullptr, flags);
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

        //collapse
        if (ImGui::Button(guiData.collapsed ? ICON_FA_CHEVRON_UP : ICON_FA_CHEVRON_DOWN, ImVec2(guiData.buttonWidth, guiData.buttonWidth))) {
            if (guiData.collapsed) {
                ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x, guiData.lastHeight));
                flags &= ~ImGuiWindowFlags_NoScrollWithMouse;
                flags &= ~ImGuiWindowFlags_NoScrollbar;
            }
            else {
                guiData.lastHeight = ImGui::GetWindowSize().y;
                flags |= ImGuiWindowFlags_NoScrollWithMouse;
                flags |= ImGuiWindowFlags_NoScrollbar;
            }
            guiData.collapsed = !guiData.collapsed;

        }
        ImGui::SameLine();

        ImGui::Text("Screen Shader");
        ImGui::SameLine(ImGui::GetWindowWidth() - (3 * guiData.buttonWidth) - 24);

        //przyciski
        if (ImGui::Button(guiData.nightMode ? ICON_FA_MOON : ICON_FA_SUN, ImVec2(guiData.buttonWidth, guiData.buttonWidth))) {
            if (guiData.nightMode) ImGui::StyleColorsLight();
            else ImGui::StyleColorsDark();
            guiData.nightMode = !guiData.nightMode;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ImVec2(guiData.buttonWidth, guiData.buttonWidth))) ShowWindow(hwnd, SW_MINIMIZE);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK, ImVec2(guiData.buttonWidth, guiData.buttonWidth))) PostQuitMessage(0);

        if (!guiData.collapsed) ImGui::Separator();
    }

    float sliderWidth = ImGui::GetWindowWidth() - guiData.labelWidth - guiData.buttonWidth - 45.0f;

    if (!guiData.collapsed) {
        ImGui::BeginChild("ContentRegion", ImVec2(0, 0), false);

        //korekcja kolorow
        if (guiData.firstFrameCC) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameCC = false;
        }
        if (ImGui::CollapsingHeader("Color corection"))
        {
            //jasnosc
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Brightness");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##brightness_slider", &shadersData.brightness, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_brightness", ImVec2(guiData.buttonWidth, 0)))
                shadersData.brightness = 1.0f;

            // gamma
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Gamma");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##gamma_slider", &shadersData.gamma, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_gamma", ImVec2(guiData.buttonWidth, 0)))
                shadersData.gamma = 1.0f;

            // kontrast
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Contrast");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##contrast_slider", &shadersData.contrast, -50.0f, 50.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_contrast", ImVec2(guiData.buttonWidth, 0)))
                shadersData.contrast = 0.0f;

            // nasycenie
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Saturation");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##saturation_slider", &shadersData.saturation, 0.0f, 3.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_saturation", ImVec2(guiData.buttonWidth, 0)))
                shadersData.saturation = 1.0f;
            ImGui::NewLine();

            //kolory rgb
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Red");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth - guiData.buttonWidth - 8.0f);
            ImGui::SliderFloat("##red_slider", &shadersData.red, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_red", ImVec2(guiData.buttonWidth, 0)))
                shadersData.red = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_red", ImVec2(guiData.buttonWidth, 0)))
                shadersData.red = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Green");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth - guiData.buttonWidth - 8.0f);
            ImGui::SliderFloat("##green_slider", &shadersData.green, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_green", ImVec2(guiData.buttonWidth, 0)))
                shadersData.green = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_green", ImVec2(guiData.buttonWidth, 0)))
                shadersData.green = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Blue");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth - guiData.buttonWidth - 8.0f);
            ImGui::SliderFloat("##blue_slider", &shadersData.blue, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_blue", ImVec2(guiData.buttonWidth, 0)))
                shadersData.blue = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_blue", ImVec2(guiData.buttonWidth, 0)))
                shadersData.blue = 1.0f;

            ImGui::NewLine();
            ImGui::NewLine();

        }


        //filtry
        if (guiData.firstFrameFIL) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameFIL = false;
        }
        if (ImGui::CollapsingHeader("Filters"))
        {
            //inwersja kolorow
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Negative");
            ImGui::SameLine();
            ImGui::Checkbox("##color_inversion_checkbox", &shadersData.colorInversion);

            //filtr czarno-bialy
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Black-White filter");
            ImGui::SameLine();
            ImGui::Checkbox("##black_white_checkbox", &shadersData.blackWhite);

            //emboss
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Emboss filter");
            ImGui::SameLine();
            ImGui::Checkbox("##emboss_checkbox", &shadersData.emboss);

            //vignette
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Vignette");
            ImGui::SameLine();
            ImGui::Checkbox("##vignette_checkbox", &shadersData.vignette);
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##vignette_radius_slider", &shadersData.vigRadius, 0.3f, 1.5f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_radius", ImVec2(guiData.buttonWidth, 0)))
                shadersData.vigRadius = 1.0f;

            ImGui::NewLine();
            ImGui::NewLine();
        }



        //odwrocenia ekranu
        if (guiData.firstFrameFL) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameFL = false;
        }
        if (ImGui::CollapsingHeader("Screen Filps"))
        {
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
            ImGui::NewLine();
        }


        //Wykrywanie krawedzi
        if (guiData.firstFrameED) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameED = false;
        }
        if (ImGui::CollapsingHeader("Edge Detection"))
        {
            //dog
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Difference of Gaussian");
            ImGui::SameLine();
            ImGui::Checkbox("##dog_checkbox", &shadersData.dog);

            /*ImGui::Text("Sigma");
            ImGui::SameLine(labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##dog_sigma_slider", &shadersData.sigma, 0.1f, 5.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_sigma", ImVec2(buttonWidth, 0)))
                shadersData.sigma = 1.0f;

            ImGui::Text("Scale - k");
            ImGui::SameLine(labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderFloat("##dog_scale_slider", &shadersData.scale, 1.0f, 5.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_scale", ImVec2(buttonWidth, 0)))
                shadersData.scale = 1.5f;*/

            ImGui::Indent(guiData.offset);
            if (ImGui::CollapsingHeader("Options"))
            {
                ImGui::Text("Brightness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(sliderWidth);
                ImGui::SliderFloat("##dog_threshold_slider", &shadersData.threshold, 0.0f, 1.0f, "%.4f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_threshold", ImVec2(guiData.buttonWidth, 0)))
                    shadersData.threshold = 0.2f;

                ImGui::Text("Sharpness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(sliderWidth);
                ImGui::SliderInt("##dog_tau_slider", &shadersData.tau, 0, 50);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_tau", ImVec2(guiData.buttonWidth, 0)))
                    shadersData.tau = 10;

                ImGui::Text("Color A");
                ImGui::SameLine(guiData.labelWidth - 26.0f);
                ImGui::ColorEdit3("##dog_color1", (float*)&shadersData.dogColor1);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor1", ImVec2(guiData.buttonWidth, 0)))
                    shadersData.dogColor1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                ImGui::Text("Color B");
                ImGui::SameLine(guiData.labelWidth - 26.0f);
                ImGui::ColorEdit3("##dog_color2", (float*)&shadersData.dogColor2);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor2", ImVec2(guiData.buttonWidth, 0)))
                    shadersData.dogColor2 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            }
            ImGui::Unindent(guiData.offset);

            ImGui::NewLine();
            ImGui::NewLine();
        }


        //Blur
        if (guiData.firstFrameB) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameB = false;
        }
        if (ImGui::CollapsingHeader("Blur"))
        {
            //blur
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Gaussian");
            ImGui::SameLine();
            ImGui::Checkbox("##blur_checkbox", &shadersData.blur);
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(sliderWidth);
            ImGui::SliderInt("##blur_radius_slider", &shadersData.blurRadius, 1, 10);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##zero_blur", ImVec2(guiData.buttonWidth, 0)))
                shadersData.blurRadius = 5;

            ImGui::NewLine();
            ImGui::NewLine();
        }
        
        
        ImGui::EndChild();
    }
    else {
        ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x, guiData.titleBarHeight));
    }


    ImVec2 imguiSize = ImGui::GetWindowSize();
    SetWindowPos(
        hwnd, nullptr, 0, 0,
        (int)imguiSize.x,(int)imguiSize.y,
        SWP_NOMOVE | SWP_NOZORDER
    );

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SwapBuffers(HDCGUI);
}

void GUI::Close() {
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