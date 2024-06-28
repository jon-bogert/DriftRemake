#include "EasyD3D.h"

template <typename T>
void SafeRelease(T* t)
{
	if (t != nullptr)
		t->Release();
}

void EasyD3D::Init(HWND hWnd, UINT constBufferSize)
{
    m_hwnd = hWnd;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = m_hwnd;
    swapChainDesc.Windowed = false;

    D3D_FEATURE_LEVEL featureLevel;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_swapChain,
        &m_device,
        &featureLevel,
        &m_context);

    assert(SUCCEEDED(hr));

    ID3D11Texture2D* framebuffer;
    hr = m_swapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&framebuffer);
    assert(SUCCEEDED(hr));

    hr = m_device->CreateRenderTargetView(
        framebuffer, 0, &m_renderTarget);
    assert(SUCCEEDED(hr));
    framebuffer->Release();

    flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* vsBlob = NULL, * psBlob = NULL, * errBlob = NULL;

    hr = D3DCompileFromFile(
        L"shader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "VS",
        "vs_5_0",
        flags,
        0,
        &vsBlob,
        &errBlob);

    if (FAILED(hr))
    {
        if (errBlob)
        {
            OutputDebugStringA((char*)errBlob->GetBufferPointer());
            errBlob->Release();
        }
        if (vsBlob)
        {
            vsBlob->Release();
        }
        assert(false);
    }

    hr = D3DCompileFromFile(
        L"shader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "PS",
        "ps_5_0",
        flags,
        0,
        &psBlob,
        &errBlob);

    if (FAILED(hr))
    {
        if (errBlob)
        {
            OutputDebugStringA((char*)errBlob->GetBufferPointer());
            errBlob->Release();
        }
        if (psBlob)
        {
            psBlob->Release();
        }
        assert(false);
    }

    hr = m_device->CreateVertexShader( vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertexShader);
    assert(SUCCEEDED(hr));

    hr = m_device->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
    assert(SUCCEEDED(hr));

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = m_device->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &m_inputLayout);
    assert(SUCCEEDED(hr));

    float vertexData[] = {
       -1.f,  1.f,  0.f,  0.f,  0.f,
        1.f,  1.f,  0.f,  1.f,  0.f,
       -1.f, -1.f,  0.f,  0.f,  1.f,

       -1.f, -1.f,  0.f,  0.f,  1.f,
        1.f,  1.f,  0.f,  1.f,  0.f,
        1.f, -1.f,  0.f,  1.f,  1.f
    };

    m_vertexStride = 5 * sizeof(float);
    m_vertexOffset = 0;
    m_vertexCount = 6;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(vertexData);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA sr_data = { 0 };
    sr_data.pSysMem = vertexData;
    hr = m_device->CreateBuffer(&vertexBufferDesc, &sr_data, &m_vertexBuffer);
    assert(SUCCEEDED(hr));

    D3D11_BUFFER_DESC shaderDataBufferDesc = {};
    shaderDataBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    shaderDataBufferDesc.ByteWidth = constBufferSize;
    shaderDataBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    shaderDataBufferDesc.CPUAccessFlags = 0;
    m_device->CreateBuffer(&shaderDataBufferDesc, nullptr, &m_shaderDataBuffer);
}

void EasyD3D::Update(const void* constBuffer)
{
    m_context->ClearRenderTargetView(m_renderTarget, &m_clearColor.r);

    D3D11_VIEWPORT viewport = Viewport();

    m_context->RSSetViewports(1, &viewport);

    m_context->OMSetRenderTargets(1, &m_renderTarget, nullptr);

    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_context->IASetInputLayout(m_inputLayout);
    m_context->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_vertexStride, &m_vertexOffset);


    m_context->UpdateSubresource(m_shaderDataBuffer, 0, nullptr, constBuffer, 0, 0);

    m_context->VSSetShader(m_vertexShader, nullptr, 0);
    m_context->PSSetShader(m_pixelShader, nullptr, 0);

    m_context->PSSetConstantBuffers(0, 1, &m_shaderDataBuffer);

    m_context->Draw(m_vertexCount, 0);

    m_swapChain->Present(1, 0);
}

EasyD3D::~EasyD3D()
{
	SafeRelease(m_device);
	SafeRelease(m_context);
	SafeRelease(m_swapChain);
	SafeRelease(m_renderTarget);
	SafeRelease(m_vertexShader);
	SafeRelease(m_pixelShader);
	SafeRelease(m_inputLayout);
	SafeRelease(m_vertexBuffer);
	SafeRelease(m_shaderDataBuffer);
}

FLOAT EasyD3D::GetWidth() const
{
    return Viewport().Width;
}

FLOAT EasyD3D::GetHeight() const
{
    return Viewport().Height;
}

D3D11_VIEWPORT EasyD3D::Viewport() const
{
    RECT winRect;
    GetClientRect(m_hwnd, &winRect);
    D3D11_VIEWPORT viewport = {
        0.0f, 0.0f,
        (FLOAT)(winRect.right - winRect.left),
        (FLOAT)(winRect.bottom - winRect.top),
        0.0f, 1.0f
    };
    return viewport;
}
