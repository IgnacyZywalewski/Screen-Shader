// full_three_window_capture.cpp
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "glad/glad.h"
#include <Windows.h>
#include <vector>
#include <cassert>

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HGLRC g_GLContextGUI = nullptr;
static HDC   g_HDCGUI = nullptr;
static HGLRC g_GLContextOverlay = nullptr;
static HDC   g_HDCOverlay = nullptr;
static bool  g_Running = true;

static GLuint screenTexture = 0;
static float  g_brightness = 1.0f;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Inicjalizacja  OpenGL
bool InitOpenGL(HWND hwnd, HDC& outHDC, HGLRC& outContext)
{
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
    if (pf == 0) return false;
    if (!SetPixelFormat(outHDC, pf, &pfd)) return false;

    outContext = wglCreateContext(outHDC);
    if (!outContext) return false;

    // ustaw jako current (bez loadowania GLAD tutaj)
    if (!wglMakeCurrent(outHDC, outContext)) return false;
    return true;
}

// shader helpers
GLuint CompileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char buf[1024]; glGetShaderInfoLog(s, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader compile error", MB_OK);
    }
    return s;
}

GLuint CreateShaderProgram(const char* vs, const char* fs)
{
    GLuint vsID = CompileShader(GL_VERTEX_SHADER, vs);
    GLuint fsID = CompileShader(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vsID);
    glAttachShader(prog, fsID);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aTexCoord");
    glLinkProgram(prog);
    GLint ok = 0; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char buf[1024]; glGetProgramInfoLog(prog, sizeof(buf), nullptr, buf);
        MessageBoxA(nullptr, buf, "Shader link error", MB_OK);
    }
    glDeleteShader(vsID);
    glDeleteShader(fsID);
    return prog;
}

void CaptureScreenToPackedBGR(std::vector<BYTE>& outPacked, int width, int height,
    HWND hwndOverlay, HWND hwndGUI, bool hideOverlayAndGuiBeforeCapture)
{
    bool overlayWasVisible = false;
    bool guiWasVisible = false;
    if (hideOverlayAndGuiBeforeCapture)
    {
        overlayWasVisible = IsWindowVisible(hwndOverlay) != FALSE;
        guiWasVisible = IsWindowVisible(hwndGUI) != FALSE;
        if (overlayWasVisible) ShowWindow(hwndOverlay, SW_HIDE);
        if (guiWasVisible)     ShowWindow(hwndGUI, SW_HIDE);

        //Sleep(30);
    }

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);

    if (!BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY))
    {
        // cleanup
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        outPacked.clear();
        if (hideOverlayAndGuiBeforeCapture)
        {
            if (overlayWasVisible) ShowWindow(hwndOverlay, SW_SHOW);
            if (guiWasVisible) ShowWindow(hwndGUI, SW_SHOW);
        }
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

    if (GetDIBits(hdcMem, hBmp, 0, height, tmp.data(), &bmi, DIB_RGB_COLORS) == 0)
    {
        SelectObject(hdcMem, hOld);
        DeleteObject(hBmp);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdcScreen);
        outPacked.clear();
        if (hideOverlayAndGuiBeforeCapture)
        {
            if (overlayWasVisible) ShowWindow(hwndOverlay, SW_SHOW);
            if (guiWasVisible) ShowWindow(hwndGUI, SW_SHOW);
        }
        return;
    }

    outPacked.resize(width * height * 3);
    for (int y = 0; y < height; ++y)
    {
        memcpy(&outPacked[y * width * 3], &tmp[y * stride], width * 3);
    }

    // cleanup GDI
    SelectObject(hdcMem, hOld);
    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    if (hideOverlayAndGuiBeforeCapture)
    {
        if (overlayWasVisible) ShowWindow(hwndOverlay, SW_SHOW);
        if (guiWasVisible) ShowWindow(hwndGUI, SW_SHOW);
        //Sleep(10);
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    const wchar_t OVERLAY_CLASS[] = L"OverlayWindowClass";
    const wchar_t GUI_CLASS[] = L"GUIWindowClass";
    const wchar_t CAPTURE_CLASS[] = L"CaptureWindowClass";

    // overlay
    WNDCLASS wc = {};
    WNDCLASS wcOverlay = {};
    wcOverlay.lpfnWndProc = OverlayWndProc;
    wcOverlay.hInstance = hInstance;
    wcOverlay.lpszClassName = OVERLAY_CLASS;
    RegisterClass(&wcOverlay);

    HWND hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        OVERLAY_CLASS, L"Overlay", WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );
    SetWindowPos(hwndOverlay, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetLayeredWindowAttributes(hwndOverlay, 0, 255, LWA_ALPHA);
    ShowWindow(hwndOverlay, SW_SHOW);

    // GUI
    WNDCLASS wcGUI = {};
    wcGUI.lpfnWndProc = GUIWndProc;
    wcGUI.hInstance = hInstance;
    wcGUI.lpszClassName = GUI_CLASS;
    RegisterClass(&wcGUI);

    HWND hwndGUI = CreateWindowEx(
        WS_EX_TOPMOST, GUI_CLASS, L"Ustawienia", WS_POPUP,
        screenWidth / 2 + 200, screenHeight / 2 - 200, 320, 420,
        nullptr, nullptr, hInstance, nullptr
    );
    ShowWindow(hwndGUI, SW_SHOW);

    SetWindowDisplayAffinity(hwndOverlay, WDA_EXCLUDEFROMCAPTURE);
    SetWindowDisplayAffinity(hwndGUI, WDA_EXCLUDEFROMCAPTURE);

    // screen capture
    WNDCLASS wcCapture = {};
    wcCapture.lpfnWndProc = CaptureWndProc;
    wcCapture.hInstance = hInstance;
    wcCapture.lpszClassName = CAPTURE_CLASS;
    RegisterClass(&wcCapture);

    HWND hwndCapture = CreateWindowEx(
        WS_EX_TOOLWINDOW, CAPTURE_CLASS, L"Capture", WS_POPUP,
        0, 0, screenWidth, screenHeight,
        nullptr, nullptr, hInstance, nullptr
    );
    SetLayeredWindowAttributes(hwndCapture, 0, 0, LWA_ALPHA);
    SetWindowPos(hwndCapture, HWND_BOTTOM, 0, 0, screenWidth, screenHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    if (!InitOpenGL(hwndOverlay, g_HDCOverlay, g_GLContextOverlay))
    {
        MessageBox(hwndOverlay, L"Init overlay OpenGL failed", L"Error", MB_OK);
        return 0;
    }

    if (!gladLoadGL())
    {
        MessageBox(hwndOverlay, L"gladLoadGL failed", L"Error", MB_OK);
        return 0;
    }

    const char* vsSrc = R"(
        #version 130
        in vec2 aPos;
        in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() { gl_Position = vec4(aPos,0,1); TexCoord = aTexCoord; }
    )";
    const char* fsSrc = R"(
        #version 130
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D screenTex;
        uniform float brightness;
        void main() {
            vec3 c = texture(screenTex, TexCoord).rgb;
            c *= brightness;
            FragColor = vec4(c,1.0);
        }
    )";

    GLuint shaderProgram = CreateShaderProgram(vsSrc, fsSrc);

    float vertices[] = {
        -1.0f,  1.0f,   0.0f, 0.0f,
        -1.0f, -1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 1.0f,
         1.0f,  1.0f,   1.0f, 0.0f
    };
    unsigned int indices[] = { 0,1,2, 0,2,3 };

    GLuint VAO = 0, VBO = 0, EBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    if (!InitOpenGL(hwndGUI, g_HDCGUI, g_GLContextGUI))
    {
        MessageBox(hwndGUI, L"Init GUI OpenGL failed", L"Error", MB_OK);
        return 0;
    }

    if (!wglMakeCurrent(g_HDCGUI, g_GLContextGUI))
    {
        MessageBox(hwndGUI, L"wglMakeCurrent GUI failed", L"Error", MB_OK);
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwndGUI);
    ImGui_ImplOpenGL3_Init("#version 130");

    std::vector<BYTE> screenPacked; // będzie trzymac width*height*3 (BGR)
    screenPacked.reserve(screenWidth * screenHeight * 3);

    CaptureScreenToPackedBGR(screenPacked, screenWidth, screenHeight, hwndOverlay, hwndGUI, /*hideOverlayAndGuiBeforeCapture=*/false);

    if (!wglMakeCurrent(g_HDCOverlay, g_GLContextOverlay))
    {
        MessageBox(hwndOverlay, L"wglMakeCurrent overlay failed", L"Error", MB_OK);
        return 0;
    }
    if (!screenPacked.empty())
    {
        glGenTextures(1, &screenTexture);
        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    DWORD lastCapture = GetTickCount64();
    const DWORD captureInterval = 8;

    MSG msg = {};
    while (g_Running)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) g_Running = false;
        }

        DWORD now = GetTickCount64();
        if (now - lastCapture >= captureInterval)
        {
            lastCapture = now;
            CaptureScreenToPackedBGR(screenPacked, screenWidth, screenHeight, hwndOverlay, hwndGUI, /*hideOverlayAndGuiBeforeCapture=*/false);

            if(wglMakeCurrent(g_HDCOverlay, g_GLContextOverlay))
            {
                if (screenPacked.empty() == false)
                {
                    if (screenTexture == 0)
                    {
                        glGenTextures(1, &screenTexture);
                        glBindTexture(GL_TEXTURE_2D, screenTexture);
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                    }
                    else
                    {
                        glBindTexture(GL_TEXTURE_2D, screenTexture);
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenWidth, screenHeight, GL_BGR, GL_UNSIGNED_BYTE, screenPacked.data());
                        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                    }
                }
            }
        }

        //render overlay
        wglMakeCurrent(g_HDCOverlay, g_GLContextOverlay);
        glViewport(0, 0, screenWidth, screenHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        if (screenTexture != 0)
        {
            glUseProgram(shaderProgram);
            glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), g_brightness);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, screenTexture);
            glUniform1i(glGetUniformLocation(shaderProgram, "screenTex"), 0);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        SwapBuffers(g_HDCOverlay);

        // render gui
        wglMakeCurrent(g_HDCGUI, g_GLContextGUI);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0)); 
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 500)); 
        ImGui::Begin("Screen Shader", nullptr, 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoScrollbar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoBringToFrontOnFocus
        ); 
        ImGui::Text("Screen Shader"); 
        ImGui::SameLine(io.DisplaySize.x - 50); 
        if (ImGui::Button("-")) ShowWindow(hwndGUI, SW_MINIMIZE); 
        ImGui::SameLine(); 
        if (ImGui::Button("X")) PostQuitMessage(0); 
        ImGui::SliderFloat("Brightness", &g_brightness, 0.0f, 2.0f); 
        
        static POINT dragOffset = { 0,0 }; 
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) { 
            POINT p; 
            GetCursorPos(&p); 
            RECT r; 
            GetWindowRect(hwndGUI, &r); 
            dragOffset.x = p.x - r.left; 
            dragOffset.y = p.y - r.top; 
        } 
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) {
            POINT p; GetCursorPos(&p); 
            SetWindowPos(hwndGUI, nullptr, p.x - dragOffset.x, p.y - dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER); 
        } 
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(g_HDCGUI);
    }

    // cleanup
    wglMakeCurrent(g_HDCGUI, g_GLContextGUI);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(g_HDCOverlay, g_GLContextOverlay);
    if (screenTexture) glDeleteTextures(1, &screenTexture);
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    wglMakeCurrent(NULL, NULL);
    if (g_GLContextGUI) wglDeleteContext(g_GLContextGUI);
    if (g_GLContextOverlay) wglDeleteContext(g_GLContextOverlay);

    if (g_HDCGUI) ReleaseDC(hwndGUI, g_HDCGUI);
    if (g_HDCOverlay) ReleaseDC(hwndOverlay, g_HDCOverlay);

    return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) return true;
    switch (uMsg)
    {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        g_Running = false;
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CaptureWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
