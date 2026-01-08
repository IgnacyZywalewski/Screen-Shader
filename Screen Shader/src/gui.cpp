#include "gui.h"

#include "pc_specs.h"
#include "data.h"
#include "helpers.h"
#include "saves.h"
#include "screen_flips.h"

#include <string>
#include <filesystem>


bool GUI::Init(HWND hwnd) {
    initThread();
  
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

void GUI::Render(HWND hwnd) {
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
        if (ImGui::Button(guiData.collapsed ? ICON_FA_CHEVRON_UP : ICON_FA_CHEVRON_DOWN, ImVec2(guiData.smallButtonSize, guiData.smallButtonSize))) {
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
        ImGui::SameLine(ImGui::GetWindowWidth() - (3 * guiData.smallButtonSize) - 24);

        //przyciski
        if (ImGui::Button(guiData.nightMode ? ICON_FA_MOON : ICON_FA_SUN, ImVec2(guiData.smallButtonSize, guiData.smallButtonSize))) {
            if (guiData.nightMode) ImGui::StyleColorsLight();
            else ImGui::StyleColorsDark();
            guiData.nightMode = !guiData.nightMode;
        }
        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_WINDOW_MINIMIZE, ImVec2(guiData.smallButtonSize, guiData.smallButtonSize))) 
            ShowWindow(hwnd, SW_MINIMIZE);

        ImGui::SameLine();

        if (ImGui::Button(ICON_FA_XMARK, ImVec2(guiData.smallButtonSize, guiData.smallButtonSize))) {
            if(guiData.currentSave != "default")
                SaveSettings(guiData.currentSave, shadersData, guiData);
            setOrientation(DMDO_DEFAULT);
            PostQuitMessage(0);
        }

        if (!guiData.collapsed) ImGui::Separator();
    }

    if (!guiData.collapsed) {
        ImGui::BeginChild("ContentRegion", ImVec2(0, guiData.contentHeight), false);

        //korekcja slepoty barw
        if (guiData.firstFrameColorBlindness) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameColorBlindness = false;
        }
        if (ImGui::CollapsingHeader("Color blindess correction")) {

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Protanopia - Red");
            ImGui::SameLine(guiData.labelWidth + 60);
            ImGui::Checkbox("##protanopia_checkbox", &shadersData.protanopia);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Option##protanopia")) {
                ImGui::Indent(guiData.offset);

                ImGui::Text("Strength");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##pronatopia_strength_slider", &shadersData.protanopiaStrength, 0.0f, 5.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_pronatopia_strength", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.protanopiaStrength = 2.0f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Deuteranopia - Green");
            ImGui::SameLine(guiData.labelWidth + 60);
            ImGui::Checkbox("##deuteranopia_checkbox", &shadersData.deuteranopia);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Option##deteranopia")) {
                ImGui::Indent(guiData.offset);

                ImGui::Text("Strength");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##deuteranopia_strength_slider", &shadersData.deuteranopiaStrength, 0.0f, 5.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_deuteranopia_strength", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.deuteranopiaStrength = 2.0f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Tritanopia - Blue");
            ImGui::SameLine(guiData.labelWidth + 60);
            ImGui::Checkbox("##tritanopia_checkbox", &shadersData.tritanopia);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Option##tritanopia")) {
                ImGui::Indent(guiData.offset);

                ImGui::Text("Strength");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##tritanopia_strength_slider", &shadersData.tritanopiaStrength, 0.0f, 3.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_tritanopia_strength", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.tritanopiaStrength = 1.5f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            ImGui::NewLine();
            ImGui::NewLine();

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Simulate Protanopia - Red");
            ImGui::SameLine(guiData.labelWidth + 130);
            ImGui::Checkbox("##simulate_protanopia_checkbox", &shadersData.simulateProtanopia);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Simulate Deuteranopia - Green");
            ImGui::SameLine(guiData.labelWidth + 130);
            ImGui::Checkbox("##simulate_deuteranopia_checkbox", &shadersData.simulateDeuteranopia);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Simulate Tritanopia - Blue");
            ImGui::SameLine(guiData.labelWidth + 130);
            ImGui::Checkbox("##simulate_tritanopia_checkbox", &shadersData.simulateTritanopia);

            ImGui::NewLine();
        }


        //odwrocenia ekranu
        if (guiData.firstFrameScreenFlips) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameScreenFlips = false;
        }
        if (ImGui::CollapsingHeader("Screen Filps")) {

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Rotate screen 90 right");
            ImGui::SameLine(guiData.labelWidth + 80);
            if (ImGui::Button("Rotate##rotate_90_right_button", ImVec2(guiData.buttonSize - 20.0f, guiData.smallButtonSize - 8.0f))) {
                rotate90right();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Rotate screen 90 left");
            ImGui::SameLine(guiData.labelWidth + 80);
            if (ImGui::Button("Rotate##rotate_90_left_button", ImVec2(guiData.buttonSize - 20.0f, guiData.smallButtonSize - 8.0f))) {
                rotate90left();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Rotate screen 180");
            ImGui::SameLine(guiData.labelWidth + 80);
            if (ImGui::Button("Rotate##rotate_180_button", ImVec2(guiData.buttonSize - 20.0f, guiData.smallButtonSize - 8.0f))) {
                rotate180();
            }

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Normal");
            ImGui::SameLine(guiData.labelWidth + 80);
            if (ImGui::Button("Normal##normal_screen_button", ImVec2(guiData.buttonSize - 20.0f, guiData.smallButtonSize - 8.0f))) {
                normalScreen();
            }

            ImGui::NewLine();

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Horizontal Swap (visual)");
            ImGui::SameLine(guiData.labelWidth + 80);
            ImGui::Checkbox("##horizontal_swap_checkbox", &shadersData.horizontalSwap);

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Vertical Swap (visual)");
            ImGui::SameLine(guiData.labelWidth + 80);
            ImGui::Checkbox("##vertical_swap_checkbox", &shadersData.verticalSwap);


            ImGui::NewLine();
        }
        

        //korekcja kolorow
        if (guiData.firstFrameColorCorection) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameColorCorection = false;
        }
        if (ImGui::CollapsingHeader("Color correction")) {
            //jasnosc
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Brightness");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##brightness_slider", &shadersData.brightness, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_brightness", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.brightness = 1.0f;

            // gamma
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Gamma");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##gamma_slider", &shadersData.gamma, 0.5f, 4.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_gamma", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.gamma = 1.0f;

            // kontrast
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Contrast");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##contrast_slider", &shadersData.contrast, -50.0f, 50.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_contrast", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.contrast = 0.0f;

            // nasycenie
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Saturation");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth);
            ImGui::SliderFloat("##saturation_slider", &shadersData.saturation, 0.0f, 3.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_saturation", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.saturation = 1.0f;
            ImGui::NewLine();

            //kolory rgb
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Red");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.smallButtonSize - 8.0f);
            ImGui::SliderFloat("##red_slider", &shadersData.red, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_red", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.red = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_red", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.red = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Green");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.smallButtonSize - 8.0f);
            ImGui::SliderFloat("##green_slider", &shadersData.green, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_green", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.green = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_green", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.green = 1.0f;

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Blue");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::PushItemWidth(guiData.sliderWidth - guiData.smallButtonSize - 8.0f);
            ImGui::SliderFloat("##blue_slider", &shadersData.blue, 0.0f, 2.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_XMARK "##zero_blue", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.blue = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_blue", ImVec2(guiData.smallButtonSize, 0)))
                shadersData.blue = 1.0f;

            ImGui::NewLine();
            ImGui::NewLine();
        }


        //filtry
        if (guiData.firstFrameFilters) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFrameFilters = false;
        }
        if (ImGui::CollapsingHeader("Filters")) {
            //tryb do czytania
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Reading mode");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##reading_mode_checkbox", &shadersData.readingMode);
            ImGui::SameLine();
            if (ImGui::CollapsingHeader("Reading Mode Option")) {
                ImGui::Indent(guiData.offset);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Temperature");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderInt("##temperature_slider", &shadersData.temperature, 2000, 5000);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_temperature", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.temperature = 4000;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

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
                ImGui::Text("Hardness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::Checkbox("##vignette_harndness_checkbox", &shadersData.vigHardness);

                ImGui::AlignTextToFramePadding();
                ImGui::Text("Radius");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##vignette_radius_slider", &shadersData.vigRadius, 0.3f, 1.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_radius", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.vigRadius = 0.75f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //film grain
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
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_amount_size", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.grainAmount = 0.5f;

                ImGui::Unindent(guiData.offset);
                ImGui::NewLine();
            }

            //wysostrzenie
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Sharpness");
            ImGui::SameLine(guiData.labelWidth);
            ImGui::Checkbox("##sharpness_checkbox", &shadersData.sharpness);

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
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_pixel_chunk_radius", ImVec2(guiData.smallButtonSize, 0)))
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
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_kuwahara_radius", ImVec2(guiData.smallButtonSize, 0)))
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
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##zero_blur", ImVec2(guiData.smallButtonSize, 0)))
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
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_threshold", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.threshold = 0.5f;

                ImGui::Text("Sharpness");
                ImGui::SameLine(guiData.labelWidth);
                ImGui::PushItemWidth(guiData.sliderWidth);
                ImGui::SliderFloat("##dog_tau_slider", &shadersData.tau, 0.5f, 50.0f, "%.2f");
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_tau", ImVec2(guiData.smallButtonSize, 0)))
                    shadersData.tau = 10.0f;

                //ImGui::Text("Color A");
                //ImGui::SameLine(guiData.labelWidth - 26.0f);
                //ImGui::ColorEdit3("##dog_color1", (float*)&shadersData.dogColor1);
                //ImGui::SameLine();
                //if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor1", ImVec2(guiData.smallButtonSize, 0)))
                //    shadersData.dogColor1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

                //ImGui::Text("Color B");
                //ImGui::SameLine(guiData.labelWidth - 26.0f);
                //ImGui::ColorEdit3("##dog_color2", (float*)&shadersData.dogColor2);
                //ImGui::SameLine();
                //if (ImGui::Button(ICON_FA_ROTATE_RIGHT "##reset_dogColor2", ImVec2(guiData.smallButtonSize, 0)))
                //    shadersData.dogColor2 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

                ImGui::Unindent(guiData.offset);
            }

            ImGui::NewLine();
            ImGui::NewLine();
        }


        //specyfikacje komputera
        if (guiData.firstFramePCSpecs) {
            ImGui::SetNextItemOpen(true);
            guiData.firstFramePCSpecs = false;
        }
        if (ImGui::CollapsingHeader("Computer Specifications")) {
            ImGui::NewLine();

            ImGui::Text("CPU: %s", GetCPUName().c_str());
            ImGui::Text("CPU overall usage: %.2f%%", GetCPUUsage());
            ImGui::Text("CPU process usage: %.2f%%", GetCPUProcessUsage());
            
            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();

            ImGui::Text("RAM available: %.2f GB", GetRAM());
            ImGui::Text("RAM overall usage: %.2f GB - %.f%%", GetRAMUsage(), (GetRAMUsage() / GetRAM()) * 100);
            ImGui::Text("RAM process usage: %.2f MB", GetRAMProcessUsage());
            
            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();
            
            ImGui::Text("Disk usage: %.2f%%", GetDiskUsage());
            ImGui::Text("Disk space total: %.2f GB", GetDiskTotalGB());
            ImGui::Text("Disk space available: %.2f GB", GetDiskTotalGB() - GetDiskUsedGB());
            
            ImGui::NewLine();
            ImGui::Separator();
            ImGui::NewLine();
            
            ImGui::Text("GPU: %s", GetGPUName().c_str());
            ImGui::Text("OpenGL version: %s", GetOpenGLVersion());

            ImGui::NewLine();
        }

        ImGui::EndChild();


        // footer
        {
            ImGui::BeginChild("Footer", ImVec2(0, guiData.footerBarHeight), false, ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::Separator();

            float centerY = (guiData.footerBarHeight - guiData.smallButtonSize - ImGui::GetStyle().ItemSpacing.y) / 2;
            ImGui::SetCursorPosY(centerY);

            float availWidth = ImGui::GetContentRegionAvail().x;
            float buttonWidth = (availWidth - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

            std::vector<std::string> saves = GetSaveList();

            if (guiData.firstFrame) {
                if (!saves.empty()) {
                    if (guiData.currentSave.empty()) {
                        guiData.currentSave = saves[0];
                        LoadSettings(guiData.currentSave);
                    }
                }
                else {
                    guiData.currentSave = "default";
                }
                guiData.firstFrame = false;
            }

            std::string comboName = guiData.currentSave != "default" ? std::string("Save: " + guiData.currentSave) : "Saves";

            ImGui::PushItemWidth(buttonWidth);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                ImVec2(8, (guiData.smallButtonSize - ImGui::GetFontSize()) * 0.5f));

            if (ImGui::BeginCombo("##saves", comboName.c_str())) {
                for (auto& file : saves) {
                    bool isSelected = (file == guiData.currentSave);
                    if (ImGui::Selectable(file.c_str(), isSelected)) {
                        if (guiData.currentSave != "default")
                            SaveSettings(guiData.currentSave, shadersData, guiData);

                        guiData.currentSave = file;
                        LoadSettings(guiData.currentSave);
                    }
                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::Separator();

                if (ImGui::Selectable("+ Add new profile")) {
                    if (guiData.currentSave != "default")
                        SaveSettings(guiData.currentSave, shadersData, guiData);

                    std::string newName = "Profile " + std::to_string(saves.size() + 1);
                    shadersData = ShadersData();
                    guiData = GUIData();

                    SaveSettings(newName, shadersData, guiData);
                    guiData.currentSave = newName;
                    saves = GetSaveList();
                }

                ImGui::EndCombo();
            }

            ImGui::PopStyleVar();
            ImGui::PopItemWidth();

            ImGui::SameLine();

            if (ImGui::Button("Delete save", ImVec2(buttonWidth, guiData.smallButtonSize))) {
            }

            ImGui::SameLine();

            if (ImGui::Button((std::string("Print Screen ") + ICON_FA_CAMERA).c_str(), ImVec2(buttonWidth, guiData.smallButtonSize))){
                SaveTextureScreenshot();
            }

            ImGui::EndChild();
        }

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
    closeThread();

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