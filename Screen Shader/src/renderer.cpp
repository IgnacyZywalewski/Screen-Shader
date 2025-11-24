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

    if (!wglMakeCurrent(HDCOverlay, GLContextOverlay))
        return false;

    if (!gladLoadGL())
        return false;

    CaptureScreenToBGR(screenPacked, screenWidth, screenHeight);

    std::string vertexShader = LoadShaderFromFile("shaders/screen_shader.vert");
    std::string fragmentShader = LoadShaderFromFile("shaders/screen_shader.frag");
    std::string kuwaharaFragShader = LoadShaderFromFile("shaders/kuwahara_shader.frag");
    std::string dogFragShader = LoadShaderFromFile("shaders/dog_shader.frag");
    std::string blurFragShader = LoadShaderFromFile("shaders/blur_shader.frag");

    shaderProgram = CreateShaderProgram(vertexShader.c_str(), fragmentShader.c_str());
    kuwaharaShaderProgram = CreateShaderProgram(vertexShader.c_str(), kuwaharaFragShader.c_str());
    dogShaderProgram = CreateShaderProgram(vertexShader.c_str(), dogFragShader.c_str());
    blurShaderProgram = CreateShaderProgram(vertexShader.c_str(), blurFragShader.c_str());

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        auto CreateFBO = [&](GLuint& tex, GLuint& fbo) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                MessageBoxA(nullptr, "FBO not complete!", "Error", MB_OK);
        };

        // kuwhara
        CreateFBO(kuwaharaTexture, kuwaharaFbo);

        // dog
        CreateFBO(dogTexture, dogFbo);

        // blur
        CreateFBO(blurTexture, blurFbo);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
    const ULONGLONG captureInterval = 4;
    ULONGLONG now = GetTickCount64();

    if (now - lastCapture >= captureInterval) {
        lastCapture = now;

        shadersData.shaderTime = now / 1000.0f;

        CaptureScreenToBGR(screenPacked, screenWidth, screenHeight);

        if (wglMakeCurrent(HDCOverlay, GLContextOverlay)) {
            if (screenTexture != 0) {
                glBindTexture(GL_TEXTURE_2D, screenTexture);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            }
        }
    }
}

void Renderer::RenderOverlay() {
    wglMakeCurrent(HDCOverlay, GLContextOverlay);

    if (screenTexture == 0) 
        return;

    //pass 1 - jasnosc, kontrast, kolory itp...
    glBindFramebuffer(GL_FRAMEBUFFER, kuwaharaFbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);
    glUniform2f(glGetUniformLocation(shaderProgram, "pixelSize"), 1.0f / screenWidth, 1.0f / screenHeight);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), shadersData.shaderTime);

    glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), shadersData.brightness);
    glUniform1f(glGetUniformLocation(shaderProgram, "gamma"), shadersData.gamma);
    glUniform1f(glGetUniformLocation(shaderProgram, "contrast"), shadersData.contrast);
    glUniform1f(glGetUniformLocation(shaderProgram, "saturation"), shadersData.saturation);
        
    glUniform1f(glGetUniformLocation(shaderProgram, "red"), shadersData.red);
    glUniform1f(glGetUniformLocation(shaderProgram, "green"), shadersData.green);
    glUniform1f(glGetUniformLocation(shaderProgram, "blue"), shadersData.blue);
        
    glUniform1i(glGetUniformLocation(shaderProgram, "colorInversion"), shadersData.colorInversion);
    glUniform1i(glGetUniformLocation(shaderProgram, "blackWhite"), shadersData.blackWhite);
    glUniform1i(glGetUniformLocation(shaderProgram, "emboss"), shadersData.emboss);

    glUniform1i(glGetUniformLocation(shaderProgram, "filmGrain"), shadersData.filmGrain);
    glUniform1f(glGetUniformLocation(shaderProgram, "grainAmount"), shadersData.grainAmount);
        
    glUniform1i(glGetUniformLocation(shaderProgram, "vignette"), shadersData.vignette);
    glUniform1f(glGetUniformLocation(shaderProgram, "vigRadius"), shadersData.vigRadius);
    glUniform1f(glGetUniformLocation(shaderProgram, "vigSmoothness"), shadersData.vigSmoothness);

    glUniform1i(glGetUniformLocation(shaderProgram, "pixel"), shadersData.pixel);
    glUniform1f(glGetUniformLocation(shaderProgram, "pixelRadius"), shadersData.pixelRadius);
        

    glUniform1i(glGetUniformLocation(shaderProgram, "horizontalSwap"), shadersData.horizontalSwap);
    glUniform1i(glGetUniformLocation(shaderProgram, "verticalSwap"), shadersData.verticalSwap);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    //pass 2 - kuwahara
    glBindFramebuffer(GL_FRAMEBUFFER, dogFbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(kuwaharaShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, kuwaharaTexture);

    glUniform1i(glGetUniformLocation(kuwaharaShaderProgram, "screenTex"), 0);
    glUniform2f(glGetUniformLocation(kuwaharaShaderProgram, "pixelSize"), 1.0f / screenWidth, 1.0f / screenHeight);

    glUniform1i(glGetUniformLocation(kuwaharaShaderProgram, "kuwahara"), shadersData.kuwahara);
    glUniform1i(glGetUniformLocation(kuwaharaShaderProgram, "kuwaharaRadius"), shadersData.kuwaharaRadius);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    //pass 3 - dog
    glBindFramebuffer(GL_FRAMEBUFFER, blurFbo);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(dogShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dogTexture);

    glUniform1i(glGetUniformLocation(dogShaderProgram, "screenTex"), 0);
    glUniform2f(glGetUniformLocation(dogShaderProgram, "pixelSize"), 1.0f / screenWidth, 1.0f / screenHeight);

    glUniform1i(glGetUniformLocation(dogShaderProgram, "dog"), shadersData.dog);
    glUniform1f(glGetUniformLocation(dogShaderProgram, "sigma"), shadersData.sigma);
    glUniform1f(glGetUniformLocation(dogShaderProgram, "scale"), shadersData.scale);
    glUniform1f(glGetUniformLocation(dogShaderProgram, "threshold"), shadersData.threshold);
    glUniform1f(glGetUniformLocation(dogShaderProgram, "tau"), shadersData.tau);
    glUniform4fv(glGetUniformLocation(dogShaderProgram, "dogColor1"), 1, (float*)&shadersData.dogColor1);
    glUniform4fv(glGetUniformLocation(dogShaderProgram, "dogColor2"), 1, (float*)&shadersData.dogColor2);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);


    //pass 4 - blur
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blurShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurTexture);

    glUniform1i(glGetUniformLocation(blurShaderProgram, "screenTex"), 0);
    glUniform2f(glGetUniformLocation(blurShaderProgram, "pixelSize"), 1.0f / screenWidth, 1.0f / screenHeight);
        
    glUniform1f(glGetUniformLocation(blurShaderProgram, "blur"), shadersData.blur);
    glUniform1i(glGetUniformLocation(blurShaderProgram, "blurRadius"), shadersData.blurRadius);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    

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
    bmi.bmiHeader.biHeight = height;
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
    if (dogTexture)
        glDeleteTextures(1, &dogTexture);
    if (blurTexture)
        glDeleteTextures(1, &blurTexture);
    
    glDeleteProgram(shaderProgram);
    glDeleteProgram(dogShaderProgram);
    glDeleteProgram(blurShaderProgram);
    
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteFramebuffers(1, &blurFbo);
    
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