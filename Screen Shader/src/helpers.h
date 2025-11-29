#include <Windows.h>
#include "data.h"

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

std::string LoadShaderFromFile(const char* path);

bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext);

bool SaveSettings(const std::string& name, const ShadersData& shader, const GUIData& gui);

bool LoadSettings(const std::string& name);

void DeleteSave(const std::string& saveName);

std::vector<std::string> GetSaveList();

void SaveTextureScreenshot();