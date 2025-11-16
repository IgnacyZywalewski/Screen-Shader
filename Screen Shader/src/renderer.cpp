#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

#include "renderer.h"
#include <glad/glad.h>
#include <Windows.h>

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

bool Renderer::Init(HWND hwndOverlay, HWND hwndGUI, int width, int height) {
    screenWidth = width;
    screenHeight = height;

    if (!InitOpenGL(hwndOverlay, HDCOverlay, GLContextOverlay))
        return false;
    
    if (!InitOpenGL(hwndGUI, HDCGUI, GLContextGUI))
        return false;
    
    if (!wglMakeCurrent(HDCOverlay, GLContextOverlay))
        return false;

    if (!wglShareLists(GLContextOverlay, GLContextGUI)) {
        MessageBox(hwndOverlay, L"Nie udało się współdzielić kontekstów OpenGL!", L"Błąd", MB_OK);
        return false;
    }
    
    if (!gladLoadGL())
        return false;

    CaptureScreenToBGR(screenPacked, screenWidth, screenHeight);
    
    std::string vertexShader = LoadShaderFromFile("shaders/screen_shader.vert");
    std::string fragmentShader = LoadShaderFromFile("shaders/screen_shader.frag");
    shaderProgram = CreateShaderProgram(vertexShader.c_str(), fragmentShader.c_str());

    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 2,  0, 2, 3 };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    if (!screenPacked.empty()) {
        glGenTextures(1, &screenTexture);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    return true;
}

bool Renderer::InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext) {
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


void Renderer::Update() {
    static ULONGLONG lastCapture = GetTickCount64();
    const ULONGLONG captureInterval = 1;
    ULONGLONG now = GetTickCount64();

    if (now - lastCapture >= captureInterval) {
        lastCapture = now;

        std::vector<BYTE> newPacked;
        if (screenTexture != 0) {
            CaptureScreenToBGR(screenPacked, screenWidth, screenHeight);

            if (wglMakeCurrent(HDCOverlay, GLContextOverlay)) {
                if (screenTexture == 0) {
                    glGenTextures(1, &screenTexture);
                    glBindTexture(GL_TEXTURE_2D, screenTexture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, screenTexture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                }
            }
        }
    }
}

void Renderer::RenderOverlay() {
    wglMakeCurrent(HDCOverlay, GLContextOverlay);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (screenTexture != 0) {
        glUseProgram(shaderProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);
        glUniform2f(glGetUniformLocation(shaderProgram, "pixelSize"), 1.0f / screenWidth, 1.0f / screenHeight);

        glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), shadersData.brightness);
        glUniform1f(glGetUniformLocation(shaderProgram, "gamma"), shadersData.gamma);
        glUniform1f(glGetUniformLocation(shaderProgram, "contrast"), shadersData.contrast);
        glUniform1f(glGetUniformLocation(shaderProgram, "saturation"), shadersData.saturation);

        glUniform1i(glGetUniformLocation(shaderProgram, "colorInversion"), shadersData.colorInversion);

        glUniform1f(glGetUniformLocation(shaderProgram, "red"), shadersData.red);
        glUniform1f(glGetUniformLocation(shaderProgram, "green"), shadersData.green);
        glUniform1f(glGetUniformLocation(shaderProgram, "blue"), shadersData.blue);

        glUniform1i(glGetUniformLocation(shaderProgram, "blackWhite"), shadersData.blackWhite);

        glUniform1i(glGetUniformLocation(shaderProgram, "horizontalSwap"), shadersData.horizontalSwap);
        glUniform1i(glGetUniformLocation(shaderProgram, "verticalSwap"), shadersData.verticalSwap);

        glUniform1i(glGetUniformLocation(shaderProgram, "blur"), shadersData.blur);
        glUniform1i(glGetUniformLocation(shaderProgram, "blurRadius"), shadersData.blurRadius);

        glUniform1f(glGetUniformLocation(shaderProgram, "emboss"), shadersData.emboss);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    SwapBuffers(HDCOverlay);
}

GLuint Renderer::CompileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char buf[1024]; glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader compile error", MB_OK);
    }
    return s;
}

GLuint Renderer::CreateShaderProgram(const char* vs, const char* fs) {
    GLuint vsID = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint fsID = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();

    glAttachShader(prog, vsID);
    glAttachShader(prog, fsID);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aTexCoord");
    glLinkProgram(prog);
    
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    
    if (!ok) {
        char buf[1024]; 
        glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader link error", MB_OK);
    }

    glDeleteShader(vsID);
    glDeleteShader(fsID);
    return prog;
}

void Renderer::CaptureScreenToBGR(std::vector<BYTE>& outPacked, int width, int height) {
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);

    if (!BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY)) {
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        outPacked.clear();
        return;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    int bytesPerPixel = 3;
    int stride = ((width * bytesPerPixel + 3) / 4) * 4;
    std::vector<BYTE> tmp(stride * height);

    if (GetDIBits(hdcMem, hBmp, 0, height, tmp.data(), &bmi, DIB_RGB_COLORS) == 0) {
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        outPacked.clear();
        return;
    }

    outPacked.resize(width * height * 3);

    for (int y = 0; y < height; y++)
        memcpy(&outPacked[y * width * 3], &tmp[y * stride], width * 3);

    SelectObject(hdcMem, hOld);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}

void Renderer::Close(HWND hwndOverlay, HWND hwndGUI) {
    wglMakeCurrent(HDCOverlay, GLContextOverlay);

    if (screenTexture)
        glDeleteTextures(1, &screenTexture);

    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    wglMakeCurrent(NULL, NULL);

    if (GLContextGUI)
        wglDeleteContext(GLContextGUI);

    if (GLContextOverlay)
        wglDeleteContext(GLContextOverlay);
    
    if (HDCGUI)
        ReleaseDC(hwndGUI, HDCGUI);
    
    if (HDCOverlay)
        ReleaseDC(hwndOverlay, HDCOverlay);
}