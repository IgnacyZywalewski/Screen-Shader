#include "gui.h"

#include <string>
#include <filesystem>


bool GUI::Init(HWND hwnd) {
    if (!InitOpenGL(hwnd, HDCGUI, GLContextGUI))
        return false;

    if (!wglMakeCurrent(HDCGUI, GLContextGUI))
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init("#version 330");

    //czcionka
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = guiData.iconFontSize;
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, guiData.iconFontSize, &icons_config, icons_ranges);

    return true;
}

void GUI::Render(HWND hwnd, Renderer& renderer) {
    wglMakeCurrent(HDCGUI, GLContextGUI);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    static ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoScrollbar;
    flags |= ImGuiWindowFlags_NoScrollWithMouse;


    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (guiData.firstFrame) {
        ImGui::SetNextWindowSize(ImVec2(guiData.windowWidth, guiData.windowHeight));
    }


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
        if (ImGui::Button(guiData.collapsed ? ICON_FA_CHEVRON_UP : ICON_FA_CHEVRON_DOWN, ImVec2(guiData.buttonSize, guiData.buttonSize))) {
            if (guiData.collapsed) {
                ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x, guiData.lastHeight));
                flags &= ~ImGuiWindowFlags_NoScrollWithMouse;
                flags &= ~ImGuiWindowFlags_NoScrollbar;
                //flags &= ~ImGuiWindowFlags_NoResize;

            }
            else {
                guiData.lastHeight = ImGui::GetWindowSize().y;
                flags |= ImGuiWindowFlags_NoScrollWithMouse;
                flags |= ImGuiWindowFlags_NoScrollbar;
                //flags |= ImGuiWindowFlags_NoResize;
            }
            guiData.collapsed = !guiData.collapsed;

        }
        ImGui::SameLine();

        ImGui::Text("Screen Shader");
        ImGui::SameLine(ImGui::GetWindowWidth() - (3 * guiData.buttonSize) - 24);

        //przyciski
        if (ImGui::Button(guiData.nightMode ? ICON_FA_MOON : ICON_FA_SUN, ImVec2(guiData.buttonSize, guiData.buttonSize))) {
            if (guiData.nightMode) ImGui::StyleColorsLight();
            else ImGui::StyleColorsDark();
            guiData.nightMode = !guiData.nightMode;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ImVec2(guiData.buttonSize, guiData.buttonSize))) ShowWindow(hwnd, SW_MINIMIZE);
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_XMARK, ImVec2(guiData.buttonSize, guiData.buttonSize))) PostQuitMessage(0);

        if (!guiData.collapsed) ImGui::Separator();
    }

    if (!guiData.collapsed) {
        ImGui::BeginChild("ContentRegion", ImVec2(0, guiData.contentHeight), false);

        //korekcja kolorow
        if (guiData.firstFrameCC) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameCC = false;
        }
        if (ImGui::CollapsingHeader("Color corection")) {
            //jasnosc
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Brightness");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##brightness_slider", &shadersData.brightness, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_brightness", ImVec2(guiData.buttonSize, 0)))
                shadersData.brightness = 1.0f;

            // gamma
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Gamma");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##gamma_slider", &shadersData.gamma, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_gamma", ImVec2(guiData.buttonSize, 0)))
                shadersData.gamma = 1.0f;

            // kontrast
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Contrast");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##contrast_slider", &shadersData.contrast, -50.0f, 50.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_contrast", ImVec2(guiData.buttonSize, 0)))
                shadersData.contrast = 0.0f;

            // nasycenie
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Saturation");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##saturation_slider", &shadersData.saturation, 0.0f, 3.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_saturation", ImVec2(guiData.buttonSize, 0)))
                shadersData.saturation = 1.0f;
            ImGui::NewLine();

            //kolory rgb
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Red");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.buttonSize - 8.0f);
            ImGui::SliderFloat("##red_slider", &shadersData.red, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_red", ImVec2(guiData.buttonSize, 0)))
                shadersData.red = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_red", ImVec2(guiData.buttonSize, 0)))
                shadersData.red = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Green");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.buttonSize - 8.0f);
            ImGui::SliderFloat("##green_slider", &shadersData.green, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_green", ImVec2(guiData.buttonSize, 0)))
                shadersData.green = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_green", ImVec2(guiData.buttonSize, 0)))
                shadersData.green = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Blue");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.buttonSize - 8.0f);
            ImGui::SliderFloat("##blue_slider", &shadersData.blue, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_blue", ImVec2(guiData.buttonSize, 0)))
                shadersData.blue = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_blue", ImVec2(guiData.buttonSize, 0)))
                shadersData.blue = 1.0f;

            ImGui::NewLine();
            ImGui::NewLine();

        }


        //filtry
        if (guiData.firstFrameFIL) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameFIL = false;
        }
        if (ImGui::CollapsingHeader("Filters")) {
            //inwersja kolorow
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Negative");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##color_inversion_checkbox", &shadersData.colorInversion);

            //filtr czarno-bialy
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Black-White");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##black_white_checkbox", &shadersData.blackWhite);

            //emboss
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Emboss");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##emboss_checkbox", &shadersData.emboss);

            //vignette
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Vignette");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##vignette_checkbox", &shadersData.vignette);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Vignette Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Radius");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##vignette_radius_slider", &shadersData.vigRadius, 0.3f, 1.5f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_radius", ImVec2(guiData.buttonSize, 0)))
                    shadersData.vigRadius = 1.0f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //grain
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Film grain");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##grain_checkbox", &shadersData.filmGrain);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Grain Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Amount");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##grain_amount_slider", &shadersData.grainAmount, 0.2f, 2.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_amount_size", ImVec2(guiData.buttonSize, 0)))
                    shadersData.grainAmount = 0.5f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //pikselizacja
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Pixelate");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##pixel_checkbox", &shadersData.pixelate);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Pixel Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Size");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderInt("##pixel_chunk_slider", &shadersData.chunk, 32, 512);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_pixel_chunk_radius", ImVec2(guiData.buttonSize, 0)))
                    shadersData.chunk = 256;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //kuwahara
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Kuwahara");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##kuwahara_checkbox", &shadersData.kuwahara);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Kuwahara Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Radius");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderInt("##kuwahara_radius_slider", &shadersData.kuwaharaRadius, 2, 5);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_kuwahara_radius", ImVec2(guiData.buttonSize, 0)))
                    shadersData.kuwaharaRadius = 2;


                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //blur
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Blur");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##blur_checkbox", &shadersData.blur);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Blur Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::Text("Radius");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderInt("##blur_radius_slider", &shadersData.blurRadius, 1, 10);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##zero_blur", ImVec2(guiData.buttonSize, 0)))
                    shadersData.blurRadius = 5;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //dog
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Difference of Gaussian");
            ImGui::SameLine();
            ImGui::Checkbox("##dog_checkbox", &shadersData.dog);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("DoG Option")) {
                ImGui::Indent(guiData.offset);
                ImGui::Text("Brightness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##dog_threshold_slider", &shadersData.threshold, 0.0f, 1.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_threshold", ImVec2(guiData.buttonSize, 0)))
                    shadersData.threshold = 0.5f;

                ImGui::Text("Sharpness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##dog_tau_slider", &shadersData.tau, 0.5f, 50.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_tau", ImVec2(guiData.buttonSize, 0)))
                    shadersData.tau = 10.0f;

                ImGui::Text("Color A");
                ImGui::SameLine(guiData.labelWidth - 26.0f);
                ImGui::ColorEdit3("##dog_color1", (float*)&shadersData.dogColor1);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor1", ImVec2(guiData.buttonSize, 0)))
                    shadersData.dogColor1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                ImGui::Text("Color B");
                ImGui::SameLine(guiData.labelWidth - 26.0f);
                ImGui::ColorEdit3("##dog_color2", (float*)&shadersData.dogColor2);
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor2", ImVec2(guiData.buttonSize, 0)))
                    shadersData.dogColor2 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            ImGui::NewLine();
            ImGui::NewLine();
        }


        //odwrocenia ekranu
        if (guiData.firstFrameFL) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameFL = false;
        }
        if (ImGui::CollapsingHeader("Screen Filps")) {
            //zamiana hotyzontalna
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Horizontal Swap");
            ImGui::SameLine(guiData.labelWidth + 20);
            ImGui::Checkbox("##horizontal_swap_checkbox", &shadersData.horizontalSwap);

            //zamiana wertykalna
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Vertical Swap");
            ImGui::SameLine(guiData.labelWidth + 20);
            ImGui::Checkbox("##vertical_swap_checkbox", &shadersData.verticalSwap);

            ImGui::NewLine();
        }

        ImGui::EndChild();


        // footer
        ImGui::BeginChild("Footer", ImVec2(0, guiData.footerBarHeight), false, ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Separator();

        float centerY = (guiData.footerBarHeight - guiData.buttonSize - ImGui::GetStyle().ItemSpacing.y) / 2;
        ImGui::SetCursorPosY(centerY);

        float buttonWidth = ImGui::GetContentRegionAvail().x / 2;

        std::vector<std::string> saves = GetSaveList();
        static std::string currentSave;

        if (guiData.firstFrame) {
            if (!saves.empty()) {
                if (currentSave.empty()) {
                    currentSave = saves[0];
                    LoadSettings(currentSave);
                }
            }
            else {
                currentSave = "default";
            }
            guiData.firstFrame = false;
        }

        std::string comboName = currentSave != "default" ? std::string("Settings: " + currentSave) : "Settings";

        ImGui::PushItemWidth(buttonWidth + 10);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, (guiData.buttonSize - ImGui::GetFontSize()) * 0.5f));
        if (ImGui::BeginCombo("##settings", comboName.c_str())) {
            for (auto& file : saves) {
                bool isSelected = (file == currentSave);

                if (ImGui::Selectable(file.c_str(), isSelected)) {
                    if (currentSave != "default") 
                        SaveSettings(currentSave, shadersData, guiData);

                    currentSave = file;
                    LoadSettings(currentSave);
                }
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::Separator();

            if (ImGui::Selectable("+ Add new profile")) {
                if (currentSave != "default")
                    SaveSettings(currentSave, shadersData, guiData);

                std::string newName = "Profile " + std::to_string(saves.size() + 1);

                shadersData = ShadersData();
                guiData = GUIData();

                SaveSettings(newName, shadersData, guiData);
                currentSave = newName;
                saves = GetSaveList();
            }

            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();

        ImGui::SameLine();

        if (ImGui::Button((std::string("Print Screen ") + ICON_FA_CAMERA).c_str(), ImVec2(buttonWidth - 10, guiData.buttonSize))) {
            renderer.SaveTextureScreenshot();
        }

        ImGui::EndChild();
    }

    else {
        ImGui::SetWindowSize(ImVec2(ImGui::GetWindowSize().x, guiData.titleBarHeight));
    }


    ImVec2 imguiSize = ImGui::GetWindowSize();
    if (!guiData.collapsed) {
        SetWindowPos(
            hwnd, nullptr, 0, 0,
            (int)imguiSize.x, (int)imguiSize.y,
            SWP_NOMOVE | SWP_NOZORDER
        );
    }
    else {
        SetWindowPos(
            hwnd, nullptr, 0, 0,
            (int)imguiSize.x, (int)guiData.titleBarHeight,
            SWP_NOMOVE | SWP_NOZORDER
        );
    }

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