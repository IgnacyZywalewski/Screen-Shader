#pragma once
#include <Windows.h>
#include <vector>
#include <glad/glad.h>

struct ShadersData {
    float brightness = 1.0f;
    float gamma = 1.0f;
    float contrast = 0.0f;
    float saturation = 1.0f;
    
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;

    bool colorInversion = false;

    bool blackWhite = false;

    bool hotizontalSwap = false;
    bool verticalSwap = false;
};

class Renderer {
public:
    bool Init(HWND hwndOverlay, HWND hwndGUI, int width, int height);
    void Update();
    void RenderOverlay();
    void Close(HWND hwndOverlay, HWND hwndGUI);
    bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext);

    ShadersData shadersData;

private:
    GLuint CompileShader(GLenum type, const char* src);
    GLuint CreateShaderProgram(const char* vs, const char* fs);
    void CaptureScreenToBGR(std::vector<BYTE>& outPacked, int width, int height);

    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;
    HGLRC GLContextOverlay = nullptr;
    HDC HDCOverlay = nullptr;

    GLuint screenTexture = 0;
    GLuint shaderProgram = 0;
    GLuint VAO = 0, VBO = 0, EBO = 0;

    std::vector<BYTE> screenPacked;

    int screenWidth = 0;
    int screenHeight = 0;
};
