#include "renderer.h"
#include <cassert>
#include <vector>
#include <glad/glad.h>
#include <Windows.h>

bool Renderer::Init(HWND hwndOverlay, HWND hwndGUI, int width, int height){
    screenWidth = width;
    screenHeight = height;

    if (!InitOpenGL(hwndOverlay, HDCOverlay, GLContextOverlay))
        return false;

    if (!gladLoadGL())
        return false;

    const char* vertexShader = R"(
        #version 130
        in vec2 aPos;
        in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() { gl_Position = vec4(aPos,0,1); TexCoord = aTexCoord; }
    )";

    const char* fragmentShader = R"(
        #version 130
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTex;
        uniform float brightness;
        uniform float contrast;
        void main() {
            vec3 c = texture(screenTex, TexCoord).rgb;
            c = (c - 0.5) * contrast + 0.5; // prosty wzór na kontrast
            c *= brightness;
            FragColor = vec4(c, 1.0);
        }
    )";

    shaderProgram = CreateShaderProgram(vertexShader, fragmentShader);

    float vertices[] = {
        -1.0f,  1.0f,   0.0f, 0.0f,
        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 0.0f
    };
    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

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

    if (!InitOpenGL(hwndGUI, HDCGUI, GLContextGUI))
        return false;

    CaptureScreenToPackedBGR(screenPacked, screenWidth, screenHeight, hwndOverlay, hwndGUI);

    if (!wglMakeCurrent(HDCOverlay, GLContextOverlay))
        return false;

    if (!screenPacked.empty()){
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

void Renderer::Update(HWND hwndOverlay, HWND hwndGUI){
    DWORD lastCapture = GetTickCount64();
    const DWORD captureInterval = 8;
    DWORD now = GetTickCount64();

    if (now - lastCapture >= captureInterval){
        lastCapture = now;
        CaptureScreenToPackedBGR(screenPacked, screenWidth, screenHeight, hwndOverlay, hwndGUI);

        if (wglMakeCurrent(HDCOverlay, GLContextOverlay)){
            if (!screenPacked.empty()){
                if (screenTexture == 0){
                    glGenTextures(1, &screenTexture);
                    glBindTexture(GL_TEXTURE_2D, screenTexture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                }
                else{
                    glBindTexture(GL_TEXTURE_2D, screenTexture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                }
            }
        }
    }
}

void Renderer::RenderOverlay(){
    wglMakeCurrent(HDCOverlay, GLContextOverlay);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    if (screenTexture != 0){
        glUseProgram(shaderProgram);
        glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), brightness);
        glUniform1f(glGetUniformLocation(shaderProgram, "contrast"), contrast);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    SwapBuffers(HDCOverlay);
}

void Renderer::Close(HWND hwndOverlay, HWND hwndGUI){
    wglMakeCurrent(HDCOverlay, GLContextOverlay);

    if (screenTexture) glDeleteTextures(1, &screenTexture);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    wglMakeCurrent(NULL, NULL);
    if (GLContextGUI) wglDeleteContext(GLContextGUI);
    if (GLContextOverlay) wglDeleteContext(GLContextOverlay);
    if (HDCGUI) ReleaseDC(hwndGUI, HDCGUI);
    if (HDCOverlay) ReleaseDC(hwndOverlay, HDCOverlay);
}

bool Renderer::InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext){
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

GLuint Renderer::CompileShader(GLenum type, const char* src){
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok){
        char buf[1024]; glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader compile error", MB_OK);
    }
    return s;
}

GLuint Renderer::CreateShaderProgram(const char* vs, const char* fs){
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
    if (!ok){
        char buf[1024]; glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader link error", MB_OK);
    }
    glDeleteShader(vsID);
    glDeleteShader(fsID);
    return prog;
}

void Renderer::CaptureScreenToPackedBGR(std::vector<BYTE>& outPacked, int width, int height, HWND hwndOverlay, HWND hwndGUI){
    bool overlayWasVisible = false;
    bool guiWasVisible = false;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);

    if (!BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY)){
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

    if (GetDIBits(hdcMem, hBmp, 0, height, tmp.data(), &bmi, DIB_RGB_COLORS) == 0){
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        outPacked.clear();
        return;
    }

    outPacked.resize(width * height * 3);
    for (int y = 0; y < height; ++y)
        memcpy(&outPacked[y * width * 3], &tmp[y * stride], width * 3);

    SelectObject(hdcMem, hOld);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
}
