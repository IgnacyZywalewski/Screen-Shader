#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "data.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

bool SaveSettings(const std::string& name, const ShadersData& shader, const GUIData& gui) {
    std::filesystem::create_directory("saves");

    json j;

    j["simulateProtanopia"] = shader.simulateProtanopia;
    j["protanopia"] = shader.protanopia;
    j["protanopiaStrength"] = shader.protanopiaStrength;

    j["simulateDeuteranopia"] = shader.simulateDeuteranopia;
    j["deuteranopia"] = shader.deuteranopia;
    j["deuteranopiaStrength"] = shader.deuteranopiaStrength;
    
    j["simulateTritanopia"] = shader.simulateTritanopia;
    j["tritanopia"] = shader.tritanopia;
    j["tritanopiaStrength"] = shader.tritanopiaStrength;

    j["brightness"] = shader.brightness;
    j["gamma"] = shader.gamma;
    j["contrast"] = shader.contrast;
    j["saturation"] = shader.saturation;

    j["red"] = shader.red;
    j["green"] = shader.green;
    j["blue"] = shader.blue;

    j["readingMode"] = shader.readingMode;
    j["temperature"] = shader.temperature;

    j["colorInversion"] = shader.colorInversion;
    j["blackWhite"] = shader.blackWhite;
    j["emboss"] = shader.emboss;

    j["vignette"] = shader.vignette;
    j["vigRadius"] = shader.vigRadius;
    j["vigHardness"] = shader.vigHardness;

    j["filmGrain"] = shader.filmGrain;
    j["grainAmount"] = shader.grainAmount;

    j["kuwahara"] = shader.kuwahara;
    j["kuwaharaRadius"] = shader.kuwaharaRadius;

    j["pixelate"] = shader.pixelate;
    j["chunk"] = shader.chunk;

    j["horizontalSwap"] = shader.horizontalSwap;
    j["verticalSwap"] = shader.verticalSwap;

    j["dog"] = shader.dog;
    j["sigma"] = shader.sigma;
    j["scale"] = shader.scale;
    j["threshold"] = shader.threshold;
    j["tau"] = shader.tau;

    j["dogColor1"] = { shader.dogColor1.x, shader.dogColor1.y, shader.dogColor1.z, shader.dogColor1.w };
    j["dogColor2"] = { shader.dogColor2.x, shader.dogColor2.y, shader.dogColor2.z, shader.dogColor2.w };

    j["blur"] = shader.blur;
    j["blurRadius"] = shader.blurRadius;

    j["sharpness"] = shader.sharpness;

    std::ofstream f("saves/" + name + ".json");
    if (!f) return false;

    f << j.dump(4);
    return true;
}

bool LoadSettings(const std::string& name) {
    std::ifstream f("saves/" + name + ".json");
    if (!f) return false;

    json j;
    f >> j;

    auto get = [&](auto& var, const char* key) {
        if (j.contains(key)) var = j[key];
    };

    get(shadersData.simulateProtanopia, "simulateProtanopia");
    get(shadersData.protanopia, "protanopia");
    get(shadersData.protanopiaStrength, "protanopiaStrength");

    get(shadersData.simulateDeuteranopia, "simulateDeuteranopia");
    get(shadersData.deuteranopia, "deuteranopia");
    get(shadersData.deuteranopiaStrength, "deuteranopiaStrength");
    
    get(shadersData.simulateTritanopia, "simulateTritanopia");
    get(shadersData.tritanopia, "tritanopia");
    get(shadersData.tritanopiaStrength, "tritanopiaStrength");

    get(shadersData.brightness, "brightness");
    get(shadersData.gamma, "gamma");
    get(shadersData.contrast, "contrast");
    get(shadersData.saturation, "saturation");

    get(shadersData.red, "red");
    get(shadersData.green, "green");
    get(shadersData.blue, "blue");

    get(shadersData.colorInversion, "colorInversion");
    get(shadersData.blackWhite, "blackWhite");
    get(shadersData.emboss, "emboss");

    get(shadersData.vignette, "vignette");
    get(shadersData.vigRadius, "vigRadius");
    get(shadersData.vigHardness, "vigHardness");

    get(shadersData.filmGrain, "filmGrain");
    get(shadersData.grainAmount, "grainAmount");

    get(shadersData.kuwahara, "kuwahara");
    get(shadersData.kuwaharaRadius, "kuwaharaRadius");

    get(shadersData.pixelate, "pixelate");
    get(shadersData.chunk, "chunk");

    get(shadersData.horizontalSwap, "horizontalSwap");
    get(shadersData.verticalSwap, "verticalSwap");

    get(shadersData.dog, "dog");
    get(shadersData.sigma, "sigma");
    get(shadersData.scale, "scale");
    get(shadersData.threshold, "threshold");
    get(shadersData.tau, "tau");

    if (j.contains("dogColor1")) {
        shadersData.dogColor1.x = j["dogColor1"][0];
        shadersData.dogColor1.y = j["dogColor1"][1];
        shadersData.dogColor1.z = j["dogColor1"][2];
        shadersData.dogColor1.w = j["dogColor1"][3];
    }

    if (j.contains("dogColor2")) {
        shadersData.dogColor2.x = j["dogColor2"][0];
        shadersData.dogColor2.y = j["dogColor2"][1];
        shadersData.dogColor2.z = j["dogColor2"][2];
        shadersData.dogColor2.w = j["dogColor2"][3];
    }

    get(shadersData.blur, "blur");
    get(shadersData.blurRadius, "blurRadius");

    get(shadersData.sharpness, "sharpness");

    return true;
}

void DeleteSave(const std::string& saveName) {
    std::string folder = "saves";
    std::string filePath = folder + "/" + saveName + ".json";

    if (std::filesystem::exists(filePath))
        std::filesystem::remove(filePath);
}

std::vector<std::string> GetSaveList() {
    std::vector<std::string> out;
    std::filesystem::create_directory("saves");

    for (auto& p : std::filesystem::directory_iterator("saves"))
    {
        if (p.path().extension() == ".json")
            out.push_back(p.path().stem().string());
    }

    return out;
}


void SaveTextureScreenshot() {
    if (lastPixels.empty()) return;

    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    std::filesystem::create_directories("screenshots");

    for (int y = 0; y < screenHeight / 2; y++) {
        for (int x = 0; x < screenWidth * 4; x++) {
            std::swap(lastPixels[y * screenWidth * 4 + x], lastPixels[(screenHeight - 1 - y) * screenWidth * 4 + x]);
        }
    }

    char filename[128];
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    std::strftime(filename, sizeof(filename), "screenshots/screenshot_%Y-%m-%d_%H-%M-%S.png", &tm);

    stbi_write_png(filename, screenWidth, screenHeight, 4, lastPixels.data(), screenWidth * 4);
}