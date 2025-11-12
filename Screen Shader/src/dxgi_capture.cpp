#include "dxgi_capture.h"
#include <windows.h>
#include <stdexcept>
#include <cassert>

DxgiCapture::DxgiCapture() {}
DxgiCapture::~DxgiCapture() { Close(); }

bool DxgiCapture::Init(int width, int height) {
    Close();
    m_width = width;
    m_height = height;

    D3D_FEATURE_LEVEL featureLevels[] = { 
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL obtained = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_device,
        &obtained,
        &m_context
    );

    if (FAILED(hr) || !m_device || !m_context) {
        OutputDebugStringA("DxgiCapture: D3D11CreateDevice failed\n");
        Close();
        return false;
    }

    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr) || !dxgiDevice) {
        OutputDebugStringA("DxgiCapture: QueryInterface IDXGIDevice failed\n");
        Close();
        return false;
    }

    IDXGIAdapter* adapter = nullptr;
    hr = dxgiDevice->GetAdapter(&adapter);
    dxgiDevice->Release();
    if (FAILED(hr) || !adapter) {
        OutputDebugStringA("DxgiCapture: GetAdapter failed\n");
        Close();
        return false;
    }

    IDXGIOutput* output = nullptr;
    hr = adapter->EnumOutputs(0, &output);
    adapter->Release();
    if (FAILED(hr) || !output) {
        OutputDebugStringA("DxgiCapture: EnumOutputs failed\n");
        Close();
        return false;
    }

    IDXGIOutput1* output1 = nullptr;
    hr = output->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1);
    output->Release();
    if (FAILED(hr) || !output1) {
        OutputDebugStringA("DxgiCapture: QueryInterface IDXGIOutput1 failed\n");
        Close();
        return false;
    }

    hr = output1->DuplicateOutput(m_device, &m_dup);
    output1->Release();
    if (FAILED(hr) || !m_dup) {
        OutputDebugStringA("DxgiCapture: DuplicateOutput failed. (Desktop Duplication not supported?)\n");
        Close();
        return false;
    }

    if (!EnsureStagingTexture(width, height)) {
        OutputDebugStringA("DxgiCapture: EnsureStagingTexture failed\n");
        Close();
        return false;
    }

    return true;
}

bool DxgiCapture::EnsureStagingTexture(int width, int height) {
    if (m_staging && m_width == width && m_height == height) 
        return true;

    if (m_staging) { 
        m_staging->Release(); 
        m_staging = nullptr; 
    }

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_staging);
    if (FAILED(hr) || !m_staging) {
        OutputDebugStringA("DxgiCapture: CreateTexture2D staging failed\n");
        return false;
    }
    m_width = width;
    m_height = height;
    return true;
}

bool DxgiCapture::AcquireFrame(std::vector<BYTE>& outPacked, UINT timeout_ms) {
    outPacked.clear();
    if (!m_dup || !m_context || !m_device) 
        return false;

    DXGI_OUTDUPL_FRAME_INFO frameInfo = {};
    IDXGIResource* desktopResource = nullptr;
    HRESULT hr = m_dup->AcquireNextFrame(timeout_ms, &frameInfo, &desktopResource);
    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false;
    }

    if (FAILED(hr)) {
        char buf[256];
        sprintf_s(buf, "DxgiCapture: AcquireNextFrame failed hr=0x%08X\n", (unsigned)hr);
        OutputDebugStringA(buf);
        if (hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_INVALID_CALL) {
            // Caller may decide to re-init the whole capture
        }
        return false;
    }

    ID3D11Texture2D* acquiredTex = nullptr;
    hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&acquiredTex);
    desktopResource->Release();
    if (FAILED(hr) || !acquiredTex) {
        OutputDebugStringA("DxgiCapture: QueryInterface ID3D11Texture2D failed\n");
        m_dup->ReleaseFrame();
        return false;
    }

    D3D11_TEXTURE2D_DESC srcDesc;
    acquiredTex->GetDesc(&srcDesc);

    if (!EnsureStagingTexture(srcDesc.Width, srcDesc.Height)) {
        acquiredTex->Release();
        m_dup->ReleaseFrame();
        return false;
    }

    m_context->CopyResource(m_staging, acquiredTex);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    hr = m_context->Map(m_staging, 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) {
        OutputDebugStringA("DxgiCapture: Map staging failed\n");
        acquiredTex->Release();
        m_dup->ReleaseFrame();
        return false;
    }

    const BYTE* src = reinterpret_cast<const BYTE*>(mapped.pData);
    int srcRowPitch = (int)mapped.RowPitch;
    int w = srcDesc.Width;
    int h = srcDesc.Height;
    outPacked.resize(w * h * 3);

    for (int y = 0; y < h; ++y) {
        const BYTE* row = src + y * srcRowPitch;
        BYTE* dst = &outPacked[y * w * 3];
        for (int x = 0; x < w; ++x) {
            dst[0] = row[x * 4 + 0]; // B
            dst[1] = row[x * 4 + 1]; // G
            dst[2] = row[x * 4 + 2]; // R
            dst += 3;
        }
    }

    m_context->Unmap(m_staging, 0);
    acquiredTex->Release();

    hr = m_dup->ReleaseFrame();
    if (FAILED(hr)) {
        OutputDebugStringA("DxgiCapture: ReleaseFrame failed\n");
    }

    return true;
}

void DxgiCapture::Close() {
    if (m_staging) { 
        m_staging->Release(); 
        m_staging = nullptr; 
    }

    if (m_dup) {
        m_dup->Release(); 
        m_dup = nullptr; 
    }

    if (m_context) { 
        m_context->Release(); 
        m_context = nullptr; 
    }

    if (m_device) { 
        m_device->Release(); 
        m_device = nullptr; 
    }
}
