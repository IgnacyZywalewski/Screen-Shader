#pragma once
#include "imgui/imgui.h"
#include <vector>
#include <string>

struct ShadersData {
    float shaderTime = 0.0f;
    
    bool simulateProtanopia = false;
    bool protanopia = false;
    float protanopiaStrength = 2.0f;

    bool simulateDeuteranopia = false;
    bool deuteranopia = false;
    float deuteranopiaStrength = 2.0f;

    bool simulateTritanopia = false;
    bool tritanopia = false;
    float tritanopiaStrength = 1.5f;

    float brightness = 1.0f;
    float gamma = 1.0f;
    float contrast = 0.0f;
    float saturation = 1.0f;

    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;

    bool readingMode = false;
    int temperature = 4000;

    bool colorInversion = false;
    bool blackWhite = false;
    bool emboss = false;

    bool vignette = false;
    float vigRadius = 0.75f;
    bool vigHardness = false;

    bool filmGrain = false;
    float grainAmount = 0.5f;

    bool kuwahara = false;
    int kuwaharaRadius = 2;

    bool pixelate = false;
    int chunk = 256;

    bool horizontalSwap = false;
    bool verticalSwap = false;

    bool dog = false;
    float sigma = 1.4f;
    float scale = 1.6f;
    float threshold = 0.5f;
    float tau = 10.0f;
    //ImVec4 dogColor1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    //ImVec4 dogColor2 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    bool blur = false;
    int blurRadius = 5;

    bool sharpness = false;
};

struct GUIData {
    std::string currentSave;

    float windowHeight = 550.0f;
    float windowWidth = 350.0f;

    float lastHeight = 0.0f;
    float titleBarHeight = 40.0f;
    float footerBarHeight = 50.0f;

    float buttonSize = 25.0f;
    float labelWidth = 90.0f;
    float offset = 10.0f;

    float sliderWidth = windowWidth - labelWidth - buttonSize - 45.0f;
    float contentHeight = windowHeight - titleBarHeight - footerBarHeight;

    float baseFontSize = 13.0f;
    float iconFontSize = baseFontSize * 2.0f / 3.0f;

    bool nightMode = true;
    bool collapsed = false;

    bool firstFrame = true;
    bool firstFrameColorBlindness = true;
    bool firstFrameColorCorection = true;
    bool firstFrameFilters = true;
    bool firstFrameScreenFlips = true;
    bool firstFramePCSpecs = true;
};

extern ShadersData shadersData;
extern GUIData guiData;
extern std::vector<unsigned char> lastPixels;