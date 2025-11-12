#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>

#include "renderer.h"
#include <glad/glad.h>
#include <Windows.h>

std::string LoadShaderFromFile(const char* path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        MessageBoxA(nullptr, (std::string("Nie można otworzyć pliku shadera: ") + path).c_str(), "Błąd", MB_OK);
        return "";
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

//void dbg(const char* fmt, ...) {
//    char buf[512];
//    va_list ap;
//    va_start(ap, fmt);
//    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
//    va_end(ap);
//    OutputDebugStringA(buf);
//    OutputDebugStringA("\n");
//}
// 
//void DumpActiveUniforms(GLuint prog) {
//    GLint n = 0;
//    glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &n);
//    dbg("Active uniforms count = %d", n);
//    char name[256];
//    for (GLint i = 0;i < n;i++) {
//        GLsizei len = 0; GLint size = 0; GLenum type = 0;
//        glGetActiveUniform(prog, i, sizeof(name), &len, &size, &type, name);
//        GLint loc = glGetUniformLocation(prog, name);
//        dbg("Uniform[%d] name='%s' len=%d size=%d type=0x%X loc=%d", i, name, len, size, type, loc);
//    }
//}

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

bool Renderer::Init(HWND hwndOverlay, HWND hwndGUI, int width, int height) {
    screenWidth = width;
    screenHeight = height;

    if (!InitOpenGL(hwndOverlay, HDCOverlay, GLContextOverlay))
        return false;
    
    if (!InitOpenGL(hwndGUI, HDCGUI, GLContextGUI))
        return false;

    if (!wglShareLists(GLContextOverlay, GLContextGUI)) {
        MessageBox(hwndOverlay, L"Nie udało się współdzielić kontekstów OpenGL!", L"Błąd", MB_OK);
        return false;
    }

    if (!gladLoadGL()) {
        MessageBox(hwndOverlay, L"Nie udało się załadować GLAD", L"Błąd", MB_OK);
        return false;
    }

    std::string vertexShader = LoadShaderFromFile("shaders/screen_shader.vert");
    std::string fragmentShader = LoadShaderFromFile("shaders/screen_shader.frag");
    shaderProgram = CreateShaderProgram(vertexShader.c_str(), fragmentShader.c_str());

    wglMakeCurrent(HDCOverlay, GLContextOverlay);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    if (!dxgi.Init(width, height)) {
        MessageBox(hwndOverlay, L"DXGI capture init failed", L"Error", MB_OK);
        return false;
    }

    screenPacked.clear();
    dxgi.AcquireFrame(screenPacked, 200);

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


void Renderer::Update() {
    static ULONGLONG lastCapture = GetTickCount64();
    const ULONGLONG captureInterval = 4;
    ULONGLONG now = GetTickCount64();

    if (now - lastCapture >= captureInterval) {
        lastCapture = now;

        std::vector<BYTE> newPacked;
        if (dxgi.AcquireFrame(newPacked, 0)) {
            screenPacked.swap(newPacked);

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
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    if (shaderProgram == 0 || VAO == 0) {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffers(HDCOverlay);
        return;
    }

    GLint linked = 0; 
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked) {
        char info[1024]; glGetProgramInfoLog(shaderProgram, sizeof(info), nullptr, info);
        SwapBuffers(HDCOverlay);
        return;
    }

    glUseProgram(shaderProgram);


    glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), shadersData.brightness);
    glUniform1f(glGetUniformLocation(shaderProgram, "gamma"), shadersData.gamma);
    glUniform1f(glGetUniformLocation(shaderProgram, "contrast"), shadersData.contrast);
    glUniform1f(glGetUniformLocation(shaderProgram, "saturation"), shadersData.saturation);

    glUniform1f(glGetUniformLocation(shaderProgram, "colorInversion"), shadersData.colorInversion);

    glUniform1f(glGetUniformLocation(shaderProgram, "red"), shadersData.red);
    glUniform1f(glGetUniformLocation(shaderProgram, "green"), shadersData.green);
    glUniform1f(glGetUniformLocation(shaderProgram, "blue"), shadersData.blue);

    glUniform1f(glGetUniformLocation(shaderProgram, "blackWhite"), shadersData.blackWhite);

    glUniform1f(glGetUniformLocation(shaderProgram, "horizontalSwap"), shadersData.horizontalSwap);
    glUniform1f(glGetUniformLocation(shaderProgram, "verticalSwap"), shadersData.verticalSwap);


    glBindVertexArray(VAO);

    if (EBO != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        GLint eboSize = 0;
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &eboSize);
        if (eboSize == 0) {
            glBindVertexArray(0);
            SwapBuffers(HDCOverlay);
            return;
        }
    }
    else {
        glBindVertexArray(0);
        SwapBuffers(HDCOverlay);
        return;
    }
   
    GLint eboBound = 0;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &eboBound);

    glUseProgram(shaderProgram);
    GLint loc = glGetUniformLocation(shaderProgram, "screenTex"); 

    if (loc >= 0) { 
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, screenTexture); 
        glUniform1i(loc, 0); 
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
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
        char buf[1024]; glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader link error", MB_OK);
    }
    glDeleteShader(vsID);
    glDeleteShader(fsID);
    return prog;
}


void Renderer::Close(HWND hwndOverlay, HWND hwndGUI){
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

    dxgi.Close();
}
