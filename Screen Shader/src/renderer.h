#pragma once
#include <Windows.h>
#include <vector>
#include <glad/glad.h>

class Renderer {
public:
    bool Init(HWND hwndOverlay, HWND hwndGUI, int width, int height);
    void Update(HWND hwndOverlay, HWND hwndGUI);
    void RenderOverlay();
    void Close(HWND hwndOverlay, HWND hwndGUI);
    bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext);

    float& GetBrightness() { return brightness; }
    float& GetContrast() { return contrast; }
    

private:
    GLuint CompileShader(GLenum type, const char* src);
    GLuint CreateShaderProgram(const char* vs, const char* fs);
    void CaptureScreenToBGR(std::vector<BYTE>& outPacked, int width, int height, HWND hwndOverlay, HWND hwndGUI);

    HGLRC GLContextGUI = nullptr;
    HDC HDCGUI = nullptr;
    HGLRC GLContextOverlay = nullptr;
    HDC HDCOverlay = nullptr;

    GLuint screenTexture = 0;
    GLuint shaderProgram = 0;
    GLuint VAO = 0, VBO = 0, EBO = 0;

    float brightness = 0.0f;
    float contrast = 0.0f;

    std::vector<BYTE> screenPacked;

    int screenWidth = 0;
    int screenHeight = 0;
};
