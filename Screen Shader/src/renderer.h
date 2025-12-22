#pragma once
#include <windows.h>

#include <glad/glad.h>
#include "imgui/imgui.h"

#include <vector>

class Renderer {
public:
    bool Init(HWND hwndOverlay, HWND hwndGUI, int width, int height);
    void Update();
    void RenderOverlay();
    void Close(HWND hwndOverlay, HWND hwndGUI);

    HGLRC GLContextOverlay = nullptr;
    HDC HDCOverlay = nullptr;

private:
    GLuint CompileShader(GLenum type, const char* src);
    GLuint CreateShaderProgram(const char* vs, const char* fs);
    void CaptureScreenToBGR(std::vector<BYTE>& outPacked, int width, int height);

    GLuint screenTexture = 0;
    GLuint colorBlindnessTexture = 0;
    GLuint sharpnessTexture = 0;
    GLuint pixelateTexture = 0;
    GLuint kuwaharaTexture = 0;
    GLuint dogTexture = 0;
    GLuint blurTexture = 0;

    GLuint shaderProgram = 0;
    GLuint colorBlindnessShaderProgram = 0;
    GLuint sharpnessShaderProgram = 0;
    GLuint pixelateShaderProgram = 0;
    GLuint kuwaharaShaderProgram = 0;
    GLuint dogShaderProgram = 0;
    GLuint blurShaderProgram = 0;

    GLuint colorBlindnessFbo = 0;
    GLuint sharpnessFbo = 0;
    GLuint pixelateFbo = 0;
    GLuint kuwaharaFbo = 0;
    GLuint blurFbo = 0;
    GLuint dogFbo = 0;
    
    GLuint VAO = 0, VBO = 0, EBO = 0;

    std::vector<unsigned char> screenPacked;

    int screenWidth = 0;
    int screenHeight = 0;
};
