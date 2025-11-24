#pragma once
#include <Windows.h>
#include <vector>
#include <glad/glad.h>
#include "imgui/imgui.h"

struct ShadersData {
    float shaderTime = 0.0f;

    float brightness = 1.0f;
    float gamma = 1.0f;
    float contrast = 0.0f;
    float saturation = 1.0f;
    
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;

    bool colorInversion = false;
    bool blackWhite = false;
    bool emboss = false;

    bool vignette = false;
    float vigRadius = 1.0f;
    float vigSmoothness = 1.0f;

    bool filmGrain = false;
    float grainAmount = 0.5f;

    bool kuwahara = false;
    int kuwaharaRadius = 2;

    bool horizontalSwap = false;
    bool verticalSwap = false;

    bool dog = false;
    float sigma = 1.4f;
    float scale = 1.5f;
    float threshold = 0.5f;
    float tau = 10.0f;
    ImVec4 dogColor1 = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 dogColor2 = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    bool blur = false;
    int blurRadius = 5;

};

class Renderer {
public:
    bool Init(HWND hwndOverlay, HWND hwndGUI, int width, int height);
    void Update();
    void RenderOverlay();
    void Close(HWND hwndOverlay, HWND hwndGUI);
    bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext);

    ShadersData shadersData;
    
    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;
    HGLRC GLContextOverlay = nullptr;
    HDC HDCOverlay = nullptr;

private:
    GLuint CompileShader(GLenum type, const char* src);
    GLuint CreateShaderProgram(const char* vs, const char* fs);
    void CaptureScreenToBGR(std::vector<BYTE>& outPacked, int width, int height);

    GLuint screenTexture = 0;
    GLuint kuwaharaTexture = 0;
    GLuint dogTexture = 0;
    GLuint blurTexture = 0;

    GLuint shaderProgram = 0;
    GLuint kuwaharaShaderProgram = 0;
    GLuint dogShaderProgram = 0;
    GLuint blurShaderProgram = 0;

    GLuint kuwaharaFbo = 0;
    GLuint blurFbo = 0;
    GLuint dogFbo = 0;
    
    GLuint VAO = 0, VBO = 0, EBO = 0;

    std::vector<BYTE> screenPacked;

    int screenWidth = 0;
    int screenHeight = 0;
};
