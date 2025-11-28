#pragma once
#include "helpers.h"

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