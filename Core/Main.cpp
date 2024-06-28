#include "EasyD3D.h"

struct ShaderData
{
    Color color1 = { 104/255.f, 107/255.f, 219/255.f };
    Color color2 = { 90/255.f, 241/255.f, 114/255.f };
    Color color3 = { 0.f, 164/255.5, 149/255.f };
    Vector2 dimensions = { 50.f, 50.f };
    float time = 0.f;
    float aspectRatio = 1.f;
    float radius = 0.0035f;
    float maxLength = 0.15f;
    float movementTimeScale = 0.1f;
    float waveTimeScale = 0.01f;
    float deadzoneFloor = 0.15f;
    float _[3];
};

// The WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// The entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, 1024, 1024 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(NULL,
        L"WindowClass",
        L"Drift",
        WS_OVERLAPPEDWINDOW,
        100,
        100,
        wr.right - wr.left,
        wr.bottom - wr.top,
        NULL,
        NULL,
        hInstance,
        NULL);

    ShowWindow(hWnd, nCmdShow);

    EasyD3D d3d;
    d3d.Init(hWnd, sizeof(ShaderData));
    ShaderData shaderData;

    MSG msg;
    while (TRUE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            break;

        shaderData.time = static_cast<float>(GetTickCount()) / 1000.0f;
        shaderData.aspectRatio = d3d.GetWidth() / d3d.GetHeight();

        d3d.Update(&shaderData);
    }

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
