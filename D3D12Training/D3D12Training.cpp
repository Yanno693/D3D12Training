#include "D3DIncludes.h"

#include "D3DRayTracingScene.h"
#include "D3DBufferManager.h"
#include "D3DShaderManager.h"
#include "D3DGenericBuffer.h"

#include "D3DRenderTarget.h"
#include "D3DVertexBuffer.h"
#include "D3DMesh.h"
#include "D3DDevice.h"
#include "D3DRenderTargetManager.h"

D3DRenderTargetManager g_D3DRenderTargetManager;

// Game Logic structures and functions

GameCamera g_Camera;
extern GameScreenResolution g_ScreenResolution;

D3DTexture* test_texture;
D3DRenderTarget* test_rt;

void InputUpdate(float const a_fDeltaTime)
{
    if (GetKeyState(VK_UP) & 0x8000)
    {
        g_Camera.transform.position.z += a_fDeltaTime;
    }
    if (GetKeyState(VK_DOWN) & 0x8000)
    {
        g_Camera.transform.position.z -= a_fDeltaTime;
    }
    if (GetKeyState(VK_RIGHT) & 0x8000)
    {
        g_Camera.transform.position.x += a_fDeltaTime;
    }
    if (GetKeyState(VK_LEFT) & 0x8000)
    {
        g_Camera.transform.position.x -= a_fDeltaTime;
    }

    if (GetKeyState('Z') & 0x8000)
    {
        g_Camera.transform.rotation.x -= a_fDeltaTime;
    }
    if (GetKeyState('S') & 0x8000)
    {
        g_Camera.transform.rotation.x += a_fDeltaTime;
    }
    if (GetKeyState('D') & 0x8000)
    {
        g_Camera.transform.rotation.y += a_fDeltaTime;
    }
    if (GetKeyState('Q') & 0x8000)
    {
        g_Camera.transform.rotation.y -= a_fDeltaTime;
    }
}

DirectX::XMMATRIX GetViewProjFromCamera(GameCamera a_oCamera, GameScreenResolution a_oResolution)
{
    using namespace DirectX;

    XMMATRIX translation = XMMatrixTranslation(
        -a_oCamera.transform.position.x,
        -a_oCamera.transform.position.y,
        -a_oCamera.transform.position.z
    );

    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
        -a_oCamera.transform.rotation.x,
        -a_oCamera.transform.rotation.y,
        -a_oCamera.transform.rotation.z
    );

    XMMATRIX View = translation * rotation;


    XMVECTOR lookAt; // Forward-oriented vector
    lookAt.m128_f32[0] = 0;
    lookAt.m128_f32[1] = 0;
    lookAt.m128_f32[2] = 1;
    XMVECTOR up; // Up-oriented vector for camera
    up.m128_f32[0] = 0;
    up.m128_f32[1] = 1;
    up.m128_f32[2] = 0;

    up = XMVector3TransformCoord(up, rotation);
    XMVECTOR EyePos;
    EyePos.m128_f32[0] = a_oCamera.transform.position.x;
    EyePos.m128_f32[1] = a_oCamera.transform.position.y;
    EyePos.m128_f32[2] = a_oCamera.transform.position.z;
    EyePos.m128_f32[3] = 1.0f;

    lookAt = XMVector3TransformCoord(lookAt, rotation) + EyePos;


    View = translation * XMMatrixLookAtLH(EyePos, lookAt, up);

    /*
    XMMATRIX Projection = XMMatrixPerspectiveLH(
        1280.0f / 1280.0f,
        //1280.0f,
        720.0f / 1280.0f,
        //720.0f,
        0.1f,
        0.1f + 50.0f
    );
    */

    XMMATRIX Projection = XMMatrixPerspectiveFovLH(
        3.14f / 2.0f, // 90°
        (float)a_oResolution.width / (float)a_oResolution.height,
        0.1f,
        0.1f + 5000.0f
    );

    XMMATRIX result = View * Projection;

    return result;
}

DirectX::XMMATRIX GetInvProjFromCamera(GameCamera a_oCamera, GameScreenResolution a_oResolution)
{
    using namespace DirectX;

    XMMATRIX Projection = XMMatrixPerspectiveFovLH(
        3.14f / 2.0f, // 90°
        (float)a_oResolution.width / (float)a_oResolution.height,
        0.1f,
        0.1f + 5000.0f
    );

    XMMATRIX InvProjection = XMMatrixInverse(nullptr, Projection);

    return InvProjection;
}

DirectX::XMMATRIX GetInvViewFromCamera(GameCamera a_oCamera)
{
    using namespace DirectX;

    XMMATRIX translation = XMMatrixTranslation(
        -a_oCamera.transform.position.x,
        -a_oCamera.transform.position.y,
        -a_oCamera.transform.position.z
    );

    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
        -a_oCamera.transform.rotation.x,
        -a_oCamera.transform.rotation.y,
        -a_oCamera.transform.rotation.z
    );

    XMMATRIX View = translation * rotation;


    XMVECTOR lookAt; // Forward-oriented vector
    lookAt.m128_f32[0] = 0;
    lookAt.m128_f32[1] = 0;
    lookAt.m128_f32[2] = 1;
    XMVECTOR up; // Up-oriented vector for camera
    up.m128_f32[0] = 0;
    up.m128_f32[1] = 1;
    up.m128_f32[2] = 0;

    up = XMVector3TransformCoord(up, rotation);
    XMVECTOR EyePos;
    EyePos.m128_f32[0] = a_oCamera.transform.position.x;
    EyePos.m128_f32[1] = a_oCamera.transform.position.y;
    EyePos.m128_f32[2] = a_oCamera.transform.position.z;
    EyePos.m128_f32[3] = 1.0f;

    lookAt = XMVector3TransformCoord(lookAt, rotation) + EyePos;


    View = translation * XMMatrixLookAtLH(EyePos, lookAt, up);

    XMMATRIX InvView = XMMatrixInverse(nullptr, View);

    return InvView;
}

std::chrono::steady_clock::time_point g_clockBegin;
std::chrono::steady_clock::time_point g_clockCurrent;
std::chrono::steady_clock::time_point g_clockLast;

float GetElapsedTime()
{
    return std::chrono::duration_cast<std::chrono::duration<float>>(g_clockCurrent - g_clockBegin).count();
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_commandAllocator;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> g_commandQueue;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> g_defaultCommandList;

Microsoft::WRL::ComPtr<ID3D12Fence> g_GPUFence;
UINT64 g_FenceValue = 1;
HANDLE g_FenceEvent;

void initD3DCommandsStructs()
{
    // Command Allocator
    if (!SUCCEEDED(D3DDevice::s_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocator))))
    {
        OutputDebugStringA("Error : Command Allocator \n");
        assert(0);
    }
    g_commandAllocator->SetName(L"D3D Command Allocator");

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC desc;
    desc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;
    if (!SUCCEEDED(D3DDevice::s_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_commandQueue))))
    {
        OutputDebugStringA("Error : Command Queue \n");
        assert(0);
    }
    g_commandQueue->SetName(L"D3D Command Queue");

    // Direct Command List 
    if (!SUCCEEDED(D3DDevice::s_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocator.Get(), NULL, IID_PPV_ARGS(&g_defaultCommandList))))
    {
        OutputDebugStringA("Error : Command List \n");
        assert(0);
    }

    g_defaultCommandList->SetName(L"D3D Default Command List");

    if (!SUCCEEDED(g_defaultCommandList->Close()))
    {
        OutputDebugStringA("Error : Command List Close \n");
        assert(0);
    }

    if (!SUCCEEDED(D3DDevice::s_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_GPUFence))))
    {
        OutputDebugStringA("Error : Fence Creation \n");
        assert(0);
    }

    g_FenceEvent = CreateEvent(NULL, FALSE, FALSE, L"GPU Fence");
    if (g_FenceEvent == nullptr)
    {
        OutputDebugStringA("Error : Fence Event Creation \n");
        assert(0);
    }
}
 
LRESULT CALLBACK m_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND g_windowHandle;
HINSTANCE g_hInstance;

void createD3DWindow(UINT const a_uiWidth, UINT const a_uiHeight)
{
    g_hInstance = GetModuleHandle(NULL);
    
    WNDCLASS wndClass = {};
    wndClass.lpfnWndProc = m_WindowProc;
    wndClass.lpszClassName = L"D3D12TrainingClass";
    wndClass.hInstance = g_hInstance;

    if (RegisterClass(&wndClass) == 0)
    {
        OutputDebugStringA("Error : Register Class when creating window \n");
        assert(0);
    }

    g_windowHandle = CreateWindow(
        L"D3D12TrainingClass",
        L"D3D12 Training",
        WS_OVERLAPPEDWINDOW | WS_CAPTION,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        a_uiWidth,
        a_uiHeight,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    ShowWindow(g_windowHandle, SW_SHOW);
}

Microsoft::WRL::ComPtr<IDXGISwapChain3> g_swapchain;
Microsoft::WRL::ComPtr<IDXGISwapChain1> g_swapchain1;

void initD3DSwapchain(UINT const a_uiWidth, UINT const a_uiHeight)
{
    DXGI_SWAP_CHAIN_DESC1 desc = {};
    desc.Width = a_uiWidth;
    desc.Height = a_uiHeight;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1; // No-Antialiasing because 1 sample, also not usable with FLIP_DISCARD ?
    desc.SampleDesc.Quality = 0;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    desc.Stereo = FALSE;

    if (!SUCCEEDED(D3DDevice::s_factory->CreateSwapChainForHwnd(g_commandQueue.Get(), g_windowHandle, &desc, NULL, NULL, &g_swapchain1)))
    {
        OutputDebugStringA("Error : Create SwapChain \n");
        assert(0);
    }

    g_swapchain1->QueryInterface(g_swapchain.GetAddressOf());
    g_swapchain1->Release();
}

D3DRenderTarget g_D3DBackBuffers[2];
UINT BackBufferIndex = 0;
UINT g_RTRSizeInHeap = 0;

void initD3DHeapStructs()
{

}

void initD3DRenderTargets()
{
    g_D3DRenderTargetManager.Initialize(D3DDevice::s_device.Get(), 1000);
    g_D3DRenderTargetManager.SetDebugName(L"Render Target Manager Decriptor Heap");
    if (!SUCCEEDED(g_swapchain->GetBuffer(0, IID_PPV_ARGS(&g_D3DBackBuffers[0].m_pResource))))
    {
        OutputDebugStringA("Error : Get Surface Buffer 0 \n");
        assert(0);
    }

    if (!SUCCEEDED(g_swapchain->GetBuffer(1, IID_PPV_ARGS(&g_D3DBackBuffers[1].m_pResource))))
    {
        OutputDebugStringA("Error : Get Surface Buffer 1 \n");
        assert(0);
    }

    g_D3DRenderTargetManager.InitializeRenderTarget(&g_D3DBackBuffers[0]);
    g_D3DRenderTargetManager.InitializeRenderTarget(&g_D3DBackBuffers[1]);

    g_D3DBackBuffers[0].SetDebugName(L"Back Buffer 0");
    g_D3DBackBuffers[1].SetDebugName(L"Back Buffer 1");
}

ULONG64 nbFrame = 0;

void WaitEndOfFrame() // TODO : Learn how fences really work
{   
    UINT64 fence = g_FenceValue;

    g_commandQueue->Signal(g_GPUFence.Get(), fence);
    g_FenceValue++;

    if (g_GPUFence->GetCompletedValue() < fence)
    {
        g_GPUFence->SetEventOnCompletion(fence, g_FenceEvent);
        WaitForSingleObject(g_FenceEvent, INFINITE);
    }
}

void RenderBegin()
{    
    nbFrame++;

    std::stringstream ss;
    ss << "Begining Frame N." << nbFrame << std::endl;
    OutputDebugStringA(ss.str().c_str());

    if (!SUCCEEDED(g_commandAllocator->Reset()))
    {
        OutputDebugStringA("Error : Command Allocator Reset \n");
        assert(0);
    }

    if (!SUCCEEDED(g_defaultCommandList->Reset(g_commandAllocator.Get(), NULL)))
    {
        OutputDebugStringA("Error : Command List Reset \n");
        assert(0);
    }
}

_SceneData g_SceneData;

void RenderLoop()
{

    /*
    FLOAT color[4] = {
        1.0f,
        0.0f,
        0.5f + (cos(GetElapsedTime() * 5.0f) * 0.5f),
        1.0f
    };
    */

    FLOAT color[4] = {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    // Update Scene data
    g_SceneConstantBuffer.TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_GENERIC_READ);
    g_SceneData.oViewProjMatrix = GetViewProjFromCamera(g_Camera, g_ScreenResolution);
    g_SceneData.oInvProjMatrix = GetInvProjFromCamera(g_Camera, g_ScreenResolution);
    g_SceneData.oInvViewMatrix = GetInvViewFromCamera(g_Camera);
    g_SceneData.oScreenSize.x = (float)g_ScreenResolution.width;
    g_SceneData.oScreenSize.y = (float)g_ScreenResolution.height;
    g_SceneConstantBuffer.WriteData(&g_SceneData, sizeof(g_SceneData));

    // Draw In Texture
    test_rt->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    g_defaultCommandList->OMSetRenderTargets(1, &test_rt->m_uiCPUHandle, false, nullptr);
    g_defaultCommandList->ClearRenderTargetView(test_rt->m_uiCPUHandle, color, 0, NULL);

    // Draw Models;
    for (UINT i = 0; i < g_MeshList.size(); i++)
    {
        //g_MeshList[i].Draw(g_defaultCommandList.Get());
        //g_MeshList[i].DrawRT(g_defaultCommandList.Get());
        g_D3DRayTracingScene.SubmitForDraw(&g_MeshList[i]);
        //g_D3DRayTracingScene.
    }

    test_rt->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    g_D3DRayTracingScene.setRenderTarget(test_rt->GetD3DTexture());

    g_D3DRayTracingScene.DrawScene(g_defaultCommandList.Get());

    // Draw In Backbuffer
    g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    g_defaultCommandList->OMSetRenderTargets(1, &g_D3DBackBuffers[BackBufferIndex].m_uiCPUHandle, false, NULL);
    g_defaultCommandList->ClearRenderTargetView(g_D3DBackBuffers[BackBufferIndex].m_uiCPUHandle, color, 0, NULL);


    // Draw Models;
    for (UINT i = 0; i < g_MeshList.size(); i++)
    {
        g_MeshList[i].Draw(g_defaultCommandList.Get());
        //g_MeshList[i].DrawRT(g_defaultCommandList.Get());

    }
    
    g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COMMON);

    // Very dirty copy into the backbuffer
    {
        test_rt->GetD3DTexture()->WaitForUAV(g_defaultCommandList.Get());
        g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
        test_rt->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);

        g_defaultCommandList.Get()->CopyResource(g_D3DBackBuffers[BackBufferIndex].m_pResource.Get(), test_texture->m_pResource.Get());
        test_rt->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COMMON);
    }


    //g_defaultCommandList.Get()->Cop
}

void RenderEnd()
{
    if (!SUCCEEDED(g_defaultCommandList->Close()))
    {
        OutputDebugStringA("Error : Command List Close \n");
        assert(0);
    }

    ID3D12CommandList* ppCommandLists[] = { g_defaultCommandList.Get() };
    g_commandQueue->ExecuteCommandLists(1, ppCommandLists);
    WaitEndOfFrame();

    g_D3DRayTracingScene.flushSceneMesh();

    if (!SUCCEEDED(g_swapchain->Present(1, 0)))
    {
        OutputDebugStringA("Error : Swapchain Present \n");
        assert(0);
    }

    BackBufferIndex = (BackBufferIndex + 1) % 2;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_SRVDescriptorHeap;
D3DVertexBuffer g_testBuffer;

int main()
{
    g_Camera.transform.position.x = 0;
    g_Camera.transform.position.y = 0;
    g_Camera.transform.position.z = -10;

    g_Camera.transform.rotation.x = 0;
    g_Camera.transform.rotation.y = 0;
    g_Camera.transform.rotation.z = 0;

    g_ScreenResolution.width = 1280;
    g_ScreenResolution.height = 720;
    
    g_clockBegin = std::chrono::steady_clock::now();

    D3DDevice::InitializeDevice();
    initD3DCommandsStructs();

    createD3DWindow(g_ScreenResolution.width, g_ScreenResolution.height);
    initD3DSwapchain(g_ScreenResolution.width, g_ScreenResolution.height);
    initD3DHeapStructs();
    initD3DRenderTargets();

    g_D3DBufferManager.Initialize(D3DDevice::s_device.Get(), 20000);
    g_D3DBufferManager.SetDebugName(L"SRV Descriptor Heap");
    g_D3DRayTracingScene.Initialize(D3DDevice::s_device.Get());

    g_D3DBufferManager.InitializeConstantBuffer(&g_SceneConstantBuffer, sizeof(_SceneData));
    g_SceneConstantBuffer.SetDebugName(L"Scene Constant Buffer");
    g_SceneData.oViewProjMatrix = DirectX::XMMatrixIdentity();

    D3DMesh oMesh;
    oMesh.Initialize("truc", D3DDevice::s_device.Get());
    //oMesh.InitializeDebug(D3DDevice::s_device.Get());
    //oMesh.InitializeDebug(D3DDevice::s_device.Get(), true);

    g_MeshList.push_back(oMesh);


    test_texture = new D3DTexture;
    test_rt = new D3DRenderTarget;
    g_D3DBufferManager.InitializeTexture(test_texture, g_ScreenResolution.width, g_ScreenResolution.height, DXGI_FORMAT_R8G8B8A8_UNORM);
    g_D3DRenderTargetManager.InitializeRenderTargetFromTexture(test_rt, test_texture);
    test_rt->SetDebugName(L"Test Texture");

    MSG message = {};

    while (message.message != WM_QUIT)
    {
        // Game Loop
        if (PeekMessage(&message, g_windowHandle, NULL, NULL, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        g_clockCurrent = std::chrono::steady_clock::now();
        float elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(g_clockCurrent - g_clockBegin).count();
        float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(g_clockCurrent - g_clockLast).count();
        g_clockLast = g_clockCurrent;

        std::stringstream ss;
        ss << "Elapsed Time : " << elapsedTime << std::endl;
        OutputDebugStringA(ss.str().c_str());

        InputUpdate(deltaTime);

        // Render Loop
        RenderBegin();
        RenderLoop();
        RenderEnd();
    }

    return 0;
}
