#include <Windows.h>

#include <string>
#include <fstream>
#include <sstream>

std::string LoadShaderFromFile(const char* path);

bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext);
