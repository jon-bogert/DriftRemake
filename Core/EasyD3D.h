#pragma once
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

struct Color
{
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 1.f;

    Color() = default;
    constexpr Color(float r, float g, float b, float a)
        : r(r), g(g), b(b), a(a) {}
    constexpr Color(float r, float g, float b)
        : r(r), g(g), b(b), a(1.f) {}
};

struct Vector3
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    Vector3() = default;
    constexpr Vector3(float x, float y, float z)
        : x(x), y(y), z(z) {}
};

struct Vector2
{
    float x = 0.f;
    float y = 0.f;

    Vector2() = default;
    constexpr Vector2(float x, float y)
        : x(x), y(y) {}
};

class EasyD3D
{
public:
    void Init(HWND hWnd, UINT constBufferSize);
    void Update(const void* constBuffer);

    EasyD3D() = default;
    ~EasyD3D();

    FLOAT GetWidth() const;
    FLOAT GetHeight() const;

private:
    D3D11_VIEWPORT Viewport() const;

	HWND m_hwnd;

	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_context = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTarget = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
	ID3D11InputLayout* m_inputLayout = nullptr;
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_shaderDataBuffer = nullptr;

	UINT m_vertexStride;
	UINT m_vertexOffset;
	UINT m_vertexCount;

	Color m_clearColor = { 0.f, 0.f, 0.f };
};