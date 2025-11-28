#pragma once
#include "helpers.h"

namespace fs = std::filesystem;


std::string LoadShaderFromFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        MessageBoxA(nullptr, (std::string("Nie mozna otworzyc pliku shadera: ") + path).c_str(), "Blad", MB_OK);
        return "";
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}


bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext) {
    outHDC = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pf = ChoosePixelFormat(outHDC, &pfd);

    if (pf == 0)
        return false;

    if (!SetPixelFormat(outHDC, pf, &pfd))
        return false;

    outContext = wglCreateContext(outHDC);

    if (!outContext)
        return false;

    if (!wglMakeCurrent(outHDC, outContext))
        return false;

    return true;
}


bool SaveSettings(const std::string& name, const ShadersData& shader, const GUIData& gui) {
    fs::create_directory("saves");
    std::ofstream f("saves/" + name + ".ini");
    if (!f) return false;

    f << "brightness " << shader.brightness << "\n";
    f << "gamma " << shader.gamma << "\n";
    f << "contrast " << shader.contrast << "\n";
    f << "saturation " << shader.saturation << "\n";

    f << "red " << shader.red << "\n";
    f << "green " << shader.green << "\n";
    f << "blue " << shader.blue << "\n";

    f << "colorInversion " << shader.colorInversion << "\n";
    f << "blackWhite " << shader.blackWhite << "\n";
    f << "emboss " << shader.emboss << "\n";
    
    f << "vignette " << shader.vignette << "\n";
    f << "vigRadius " << shader.vigRadius << "\n";
    f << "vigSmoothness " << shader.vigSmoothness << "\n";

    f << "filmGrain " << shader.filmGrain << "\n";
    f << "grainAmount " << shader.grainAmount << "\n";

    f << "kuwahara " << shader.kuwahara << "\n";
    f << "kuwaharaRadius " << shader.kuwaharaRadius << "\n";
    
    f << "pixelate " << shader.pixelate << "\n";
    f << "chunk " << shader.chunk << "\n";
    
    f << "horizontalSwap " << shader.horizontalSwap << "\n";
    f << "verticalSwap " << shader.verticalSwap << "\n";

    f << "dog " << shader.dog << "\n";
    f << "sigma " << shader.sigma << "\n";
    f << "scale " << shader.scale << "\n";
    f << "threshold " << shader.threshold << "\n";
    f << "tau " << shader.tau << "\n";

    f << "dogColor1 "
        << shader.dogColor1.x << " "
        << shader.dogColor1.y << " "
        << shader.dogColor1.z << " "
        << shader.dogColor1.w << "\n";

    f << "dogColor2 "
        << shader.dogColor2.x << " "
        << shader.dogColor2.y << " "
        << shader.dogColor2.z << " "
        << shader.dogColor2.w << "\n";

    f << "blur " << shader.blur << "\n";
    f << "blurRadius " << shader.blurRadius << "\n";

    return true;
}

bool LoadSettings(const std::string& name) {
    std::ifstream f("saves/" + name + ".ini");
    if (!f) return false;

    std::string k;
    while (f >> k)
    {
        if (k == "brightness") f >> shadersData.brightness;
        else if (k == "gamma") f >> shadersData.gamma;
        else if (k == "contrast") f >> shadersData.contrast;
        else if (k == "saturation") f >> shadersData.saturation;

        else if (k == "red") f >> shadersData.red;
        else if (k == "green") f >> shadersData.green;
        else if (k == "blue") f >> shadersData.blue;

        else if (k == "colorInversion") f >> shadersData.colorInversion;
        else if (k == "blackWhite") f >> shadersData.blackWhite;
        else if (k == "emboss") f >> shadersData.emboss;

        else if (k == "vignette") f >> shadersData.vignette;
        else if (k == "vigRadius") f >> shadersData.vigRadius;
        else if (k == "vigSmoothness") f >> shadersData.vigSmoothness;

        else if (k == "filmGrain") f >> shadersData.filmGrain;
        else if (k == "grainAmount") f >> shadersData.grainAmount;

        else if (k == "kuwahara") f >> shadersData.kuwahara;
        else if (k == "kuwaharaRadius") f >> shadersData.kuwaharaRadius;

        else if (k == "pixelate") f >> shadersData.pixelate;
        else if (k == "chunk") f >> shadersData.chunk;

        else if (k == "horizontalSwap") f >> shadersData.horizontalSwap;
        else if (k == "verticalSwap") f >> shadersData.verticalSwap;

        else if (k == "dog") f >> shadersData.dog;
        else if (k == "sigma") f >> shadersData.sigma;
        else if (k == "scale") f >> shadersData.scale;
        else if (k == "threshold") f >> shadersData.threshold;
        else if (k == "tau") f >> shadersData.tau;

        else if (k == "dogColor1")
            f >> shadersData.dogColor1.x
            >> shadersData.dogColor1.y
            >> shadersData.dogColor1.z
            >> shadersData.dogColor1.w;

        else if (k == "dogColor2")
            f >> shadersData.dogColor2.x
            >> shadersData.dogColor2.y
            >> shadersData.dogColor2.z
            >> shadersData.dogColor2.w;

        else if (k == "blur") f >> shadersData.blur;
        else if (k == "blurRadius") f >> shadersData.blurRadius;

    }

    return true;
}

void DeleteSave(const std::string& saveName) {
    std::string folder = "saves";
    std::string filePath = folder + "/" + saveName + ".ini";

    if (fs::exists(filePath))
        fs::remove(filePath);
}

std::vector<std::string> GetSaveList() {
    std::vector<std::string> out;
    fs::create_directory("saves");

    for (auto& p : fs::directory_iterator("saves"))
    {
        if (p.path().extension() == ".ini")
            out.push_back(p.path().stem().string());
    }
    return out;
}

std::string LoadLastSave() {
    std::ifstream file("saves/last_save.txt");
    std::string lastSave;
    if (file.is_open()) {
        std::getline(file, lastSave);
        file.close();
    }
    if (lastSave.empty()) return "default";
    return lastSave;
}

void SaveLastSave(const std::string& saveName) {
    std::ofstream file("saves/last_save.txt", std::ios::trunc);
    if (file.is_open()) {
        file << saveName;
        file.close();
    }
}