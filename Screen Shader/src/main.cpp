#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h" 
#include "imgui/backends/imgui_impl_win32.h" 
#include "glad/glad.h" 
#include <Windows.h> 

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static HGLRC g_GLContext = nullptr;
static HDC g_HDC = nullptr;
static bool g_Running = true;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool InitOpenGL(HWND hwnd) {
	g_HDC = GetDC(hwnd);

	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;

	int pf = ChoosePixelFormat(g_HDC, &pfd);
	if (pf == 0) return false;
	if (!SetPixelFormat(g_HDC, pf, &pfd)) return false;
	g_GLContext = wglCreateContext(g_HDC);
	if (!g_GLContext) return false;
	wglMakeCurrent(g_HDC, g_GLContext);
	if (!gladLoadGL()) { 
		MessageBox(hwnd, L"Nie udało się załadować GLAD.", L"Błąd", MB_OK); 
		return false; 
	} 
	return true;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	const wchar_t OVERLAY_CLASS[] = L"OverlayWindowClass";
	const wchar_t GUI_CLASS[] = L"GUIWindowClass"; 
	
	WNDCLASS wcOverlay = {};
	wcOverlay.lpfnWndProc = OverlayWndProc;
	wcOverlay.hInstance = hInstance;
	wcOverlay.lpszClassName = OVERLAY_CLASS;
	wcOverlay.hbrBackground = nullptr;

	RegisterClass(&wcOverlay);


	WNDCLASS wcGUI = {};
	wcGUI.lpfnWndProc = GUIWndProc;
	wcGUI.hInstance = hInstance;
	wcGUI.lpszClassName = GUI_CLASS;
	wcGUI.hbrBackground = nullptr;

	RegisterClass(&wcGUI);


	HWND hwndOverlay = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, 
		OVERLAY_CLASS,
		L"Overlay", 
		WS_POPUP, 0, 0, screenWidth, screenHeight, 
		nullptr, nullptr, hInstance, nullptr
	);

	SetLayeredWindowAttributes(hwndOverlay, 0, 255, LWA_ALPHA);
	ShowWindow(hwndOverlay, SW_SHOW);


	HWND hwndGUI = CreateWindowEx(
		0, 
		GUI_CLASS, 
		L"Ustawienia Filtra", 
		WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 300, 500, 
		nullptr, nullptr, hInstance, nullptr
	);

	MoveWindow(hwndGUI, screenWidth / 2.0 + 300.0, screenHeight / 2.0 - 300.0, 300, 500, TRUE);
	ShowWindow(hwndGUI, SW_SHOW);

	if (!InitOpenGL(hwndGUI)) { 
		MessageBox(hwndGUI, L"Nie udało się zainicjalizować OpenGL!", L"Błąd", MB_OK);
		return 0; 
	} 

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(hwndGUI);
	ImGui_ImplOpenGL3_Init("#version 130");

	MSG msg = {};
	while (g_Running) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) g_Running = false;
		}
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
		if (ImGui::Button("-")) { 
			ShowWindow(hwndGUI, SW_MINIMIZE); 
		}
		ImGui::SameLine();
		if (ImGui::Button("X")) { 
			PostQuitMessage(0); 
		}
		static POINT dragOffset = { 0, 0 };
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) {
			POINT p; 
			GetCursorPos(&p); 
			RECT r; 
			GetWindowRect(hwndGUI, &r);
			dragOffset.x = p.x - r.left; 
			dragOffset.y = p.y - r.top;
		}
		if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) {
			POINT p; 
			GetCursorPos(&p);
			SetWindowPos(hwndGUI, nullptr, p.x - dragOffset.x, p.y - dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		ImGui::End();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y); 
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SwapBuffers(g_HDC);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(g_GLContext);
	ReleaseDC(hwndGUI, g_HDC); return 0;
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_PAINT:
		break;
	case WM_DESTROY: 
		return 0;
	} 
	return DefWindowProc(hwnd, uMsg, wParam, lParam); 
} 

LRESULT CALLBACK GUIWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) { 
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam)) return true; 
	switch (uMsg) { 
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