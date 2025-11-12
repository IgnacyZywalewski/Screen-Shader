#pragma once
#include <d3d11.h>
#include <dxgi1_2.h>
#include <vector>

class DxgiCapture {
public:
    DxgiCapture();
    ~DxgiCapture();

    bool Init(int width, int height);
    bool AcquireFrame(std::vector<BYTE>& outPacked, UINT timeout_ms = 100);
    void Close();

private:
    bool EnsureStagingTexture(int width, int height);

    ID3D11Device* m_device = nullptr;
    ID3D11DeviceContext* m_context = nullptr;
    IDXGIOutputDuplication* m_dup = nullptr;
    ID3D11Texture2D* m_staging = nullptr;

    int m_width = 0;
    int m_height = 0;
};
