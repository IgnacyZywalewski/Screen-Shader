#include "helpers.h"

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