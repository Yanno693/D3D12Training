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

#include "ShaderShared.h"
#include "GameScene.h"

#include <Xinput.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

D3DRenderTargetManager g_D3DRenderTargetManager;

// Game Logic structures and functions

GameCamera g_Camera;

D3DTexture* test_texture;
D3DTexture* test_depth;
D3DRenderTarget* mainRT;
D3DDepthBuffer* mainDepth;

ULONG64 nbFrame = 0;
bool bImGUIInitialized = false;

DirectX::XMVECTOR up{ 0.0f, 1, 0, 0 };

bool g_bIsRunning = true;

void InputUpdate(float a_fDeltaTime)
{
    using namespace DirectX;

    a_fDeltaTime *= 5;
    
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
    if (GetKeyState(VK_CONTROL) & 0x8000)
    {
        g_Camera.transform.position.y -= a_fDeltaTime;
    }
    if (GetKeyState(VK_SPACE) & 0x8000)
    {
        g_Camera.transform.position.y += a_fDeltaTime;
    }

    if (GetKeyState('D') & 0x8000)
    {
        g_Camera.transform.rotation.y += a_fDeltaTime;
        //g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationY(-a_fDeltaTime * 0.2);
        g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(up, -a_fDeltaTime * 0.2f);
    }
    if (GetKeyState('Q') & 0x8000)
    {
        g_Camera.transform.rotation.y -= a_fDeltaTime;
        //g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationY(a_fDeltaTime * 0.2);
        g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(up, a_fDeltaTime * 0.2f);
    }

    XMVECTOR right(g_Camera.transform.rotation.rotationMatrix.r[0]);
    XMVECTOR forward(g_Camera.transform.rotation.rotationMatrix.r[2]);

    if (GetKeyState('Z') & 0x8000)
    {
        g_Camera.transform.rotation.x -= a_fDeltaTime;
        //g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationX(a_fDeltaTime * 0.2);
        g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(right, a_fDeltaTime * 0.2f);
    }
    if (GetKeyState('S') & 0x8000)
    {
        g_Camera.transform.rotation.x += a_fDeltaTime;
        //g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationX(-a_fDeltaTime * 0.2);
        g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(right, -a_fDeltaTime * 0.2f);
    }

    DWORD dwResult;
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));
    dwResult = XInputGetState(0, &state);

    if (dwResult == ERROR_SUCCESS)
    {
        float RX = state.Gamepad.sThumbRX;
        float RY = state.Gamepad.sThumbRY;
        float LX = state.Gamepad.sThumbLX;
        float LY = state.Gamepad.sThumbLY;

        float deadzone = sqrt(RX * RX + RY * RY);
        if (deadzone > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        {
            g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(up, RX * a_fDeltaTime * 0.00002f);
            g_Camera.transform.rotation.rotationMatrix *= DirectX::XMMatrixRotationAxis(right, -RY * a_fDeltaTime * 0.00002f);
        }

        deadzone = sqrt(LX * LX + LY * LY);
        if (deadzone > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        {
            XMMATRIX forwardTranslation = XMMatrixTranslationFromVector(forward) * LY * 0.00001f;
            XMMATRIX rightTranslation = XMMatrixTranslationFromVector(right) * LX * 0.00001f;

            XMMATRIX finalTranslation = forwardTranslation + rightTranslation;

            g_Camera.transform.position.x += finalTranslation.r[3].m128_f32[0];
            g_Camera.transform.position.y += finalTranslation.r[3].m128_f32[1];
            g_Camera.transform.position.z += finalTranslation.r[3].m128_f32[2];
        }
        //printf("Forward : X %f Y %f\n", RX, RY);
    }

    //printf("Forward : %f %f %f %f\n", forward.m128_f32[0], forward.m128_f32[1], forward.m128_f32[2], forward.m128_f32[3]);
    //printf("Right : %f %f %f %f\n", right.m128_f32[0], right.m128_f32[1], right.m128_f32[2], right.m128_f32[3]);
    //printf("Position : X=%f Y=%f Z=%f\n", g_Camera.transform.position.x, g_Camera.transform.position.y, g_Camera.transform.position.z);
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

    XMMATRIX View = translation * XMMatrixTranspose(a_oCamera.transform.rotation.rotationMatrix);

    XMMATRIX Projection = XMMatrixPerspectiveFovLH(
        3.14f / 2.0f, // 90°
        (float)a_oResolution.width / (float)a_oResolution.height,
        0.5f,
        0.5f + 5000.0f
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
        0.5f,
        0.5f + 5000.0f
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

    XMMATRIX View = translation * XMMatrixTranspose(a_oCamera.transform.rotation.rotationMatrix);

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
    g_GPUFence.Get()->SetName(L"D3D GPU Fence");

    g_FenceEvent = CreateEvent(NULL, FALSE, FALSE, L"GPU Fence");
    if (g_FenceEvent == nullptr)
    {
        OutputDebugStringA("Error : Fence Event Creation \n");
        assert(0);
    }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 
LRESULT CALLBACK m_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;
    
    if (uMsg == WM_CLOSE)
    {
        PostQuitMessage(0);
    }

    if (uMsg == WM_SIZE)
    {
        if (bImGUIInitialized)
        {
            ImGuiIO& roImGUIIO = ImGui::GetIO();
            /*
            roImGUIIO.
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            roImGUIIO.DisplaySize.x = width + 1000;
            roImGUIIO.DisplaySize.y = height + 1000;
            */
        }
    }
    
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
    
    // https://stackoverflow.com/questions/61975605/createwindoww-window-is-too-small
    // The size provided to CreateWindow includes the menu bar and stuff like that, thus needing to adjust the size to truly match
    RECT oWindowRect = { 0, 0, a_uiWidth, a_uiHeight };
    AdjustWindowRect(&oWindowRect, WS_OVERLAPPEDWINDOW | WS_CAPTION, true);

    g_windowHandle = CreateWindow(
        L"D3D12TrainingClass",
        L"D3D12 Training",
        WS_OVERLAPPEDWINDOW | WS_CAPTION,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        oWindowRect.right - oWindowRect.left,
        oWindowRect.bottom - oWindowRect.top,
        NULL,
        NULL,
        g_hInstance,
        NULL
    );

    ShowWindow(g_windowHandle, SW_SHOW);
}

D3DRenderTarget g_D3DBackBuffers[2];
UINT BackBufferIndex = 0;

void initD3DRenderTargets()
{
    g_D3DRenderTargetManager.Initialize(D3DDevice::s_device.Get(), 1000);
    g_D3DRenderTargetManager.SetDebugName(L"Render Target Manager Decriptor Heap");
    if (!SUCCEEDED(D3DDevice::s_swapchain->GetBuffer(0, IID_PPV_ARGS(&g_D3DBackBuffers[0].m_pResource))))
    {
        OutputDebugStringA("Error : Get Surface Buffer 0 \n");
        assert(0);
    }

    if (!SUCCEEDED(D3DDevice::s_swapchain->GetBuffer(1, IID_PPV_ARGS(&g_D3DBackBuffers[1].m_pResource))))
    {
        OutputDebugStringA("Error : Get Surface Buffer 1 \n");
        assert(0);
    }

    g_D3DRenderTargetManager.InitializeRenderTarget(&g_D3DBackBuffers[0]);
    g_D3DRenderTargetManager.InitializeRenderTarget(&g_D3DBackBuffers[1]);

    g_D3DBackBuffers[0].SetDebugName(L"Back Buffer 0");
    g_D3DBackBuffers[1].SetDebugName(L"Back Buffer 1");
}

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

void InitializeImGUI()
{
    ImGui::CreateContext();

    ImGuiIO& roImGUIIO = ImGui::GetIO();
    roImGUIIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad; // Enable Keyboard Controls


    ImGui_ImplDX12_InitInfo oImGUIInitInfo = {};
    oImGUIInitInfo.Device = D3DDevice::s_device.Get();
    oImGUIInitInfo.CommandQueue = g_commandQueue.Get();
    oImGUIInitInfo.NumFramesInFlight = 1; // ?
    oImGUIInitInfo.RTVFormat = BACK_BUFFER_FORMAT;
    oImGUIInitInfo.DSVFormat = DEPTH_BUFFER_FORMAT;
    oImGUIInitInfo.SrvDescriptorHeap = g_D3DBufferManager.GetHeap();
    oImGUIInitInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {};
    oImGUIInitInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
    {
        g_D3DBufferManager.RequestDescriptor(*out_cpu_handle, *out_gpu_handle);
    };

    ImGui_ImplDX12_Init(&oImGUIInitInfo);
    ImGui_ImplWin32_Init(g_windowHandle);

    bImGUIInitialized = true;
}

// Generate content to draw
void DrawImGUI()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();
}

// Draw to back buffer
void RenderImGUI()
{
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_defaultCommandList.Get());
}

void RenderBegin()
{    
    nbFrame++;

    std::stringstream ss;
    ss << "Beginning Frame N." << nbFrame << std::endl;
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

void RenderLoop()
{
    FLOAT color[4] = {
        0.0f,
        0.0f,
        0.0f,
        0.0f
    };

    // Update Scene data
    g_GameScene.m_pSceneConstantBuffer.TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
    g_GameScene.m_oSceneData.oViewProjMatrix = GetViewProjFromCamera(g_Camera, g_ScreenResolution);
    g_GameScene.m_oSceneData.oInvProjMatrix = GetInvProjFromCamera(g_Camera, g_ScreenResolution);
    g_GameScene.m_oSceneData.oInvViewMatrix = GetInvViewFromCamera(g_Camera);
    g_GameScene.m_oSceneData.oCameraWorldPos = g_Camera.transform.position;
    g_GameScene.m_oSceneData.oScreenSize.x = (float)g_ScreenResolution.width;
    g_GameScene.m_oSceneData.oScreenSize.y = (float)g_ScreenResolution.height;
    g_GameScene.m_oSceneData.uiPointLightCount = g_GameScene.GetPointLightsCount();
    g_GameScene.m_pSceneConstantBuffer.WriteData(&g_GameScene.m_oSceneData, sizeof(g_GameScene.m_oSceneData));
    g_GameScene.m_oSceneData.oDirectionalLight.angle.z = (cos(GetElapsedTime() * 0.5f) * 1.5f); // TODO : Normalize the angle

    if (D3DDevice::isRayTracingEnabled())
    {
        // Submit model to draw, 
        for (UINT i = 0; i < D3DMesh::s_MeshList.size(); i++)
        {
            g_D3DRayTracingScene.SubmitForDraw(&D3DMesh::s_MeshList[i]);
        }

        mainRT->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        g_D3DRayTracingScene.setRenderTarget(mainRT->GetD3DTexture());

        // Draw
        g_D3DRayTracingScene.DrawScene(g_defaultCommandList.Get());
        
        // Very dirty copy into the backbuffer
        mainRT->GetD3DTexture()->WaitForUAV(g_defaultCommandList.Get());
        g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
        mainRT->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);

        g_defaultCommandList.Get()->CopyResource(g_D3DBackBuffers[BackBufferIndex].m_pResource.Get(), test_texture->m_pResource.Get());
        mainRT->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COMMON);
    }
    else
    {
        // Draw In Backbuffer
        g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        mainDepth->TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE);

        g_defaultCommandList->OMSetRenderTargets(1, &g_D3DBackBuffers[BackBufferIndex].m_uiCPUHandle, false, &mainDepth->m_uiCPUHandle);
        g_defaultCommandList->ClearRenderTargetView(g_D3DBackBuffers[BackBufferIndex].m_uiCPUHandle, color, 0, NULL);
        g_defaultCommandList->ClearDepthStencilView(mainDepth->m_uiCPUHandle, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, NULL);

        // Draw Models;
        for (UINT i = 0; i < D3DMesh::s_MeshList.size(); i++)
        {
            D3DMesh::s_MeshList[i].Draw(g_defaultCommandList.Get());
        }
    }

    g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    g_defaultCommandList->OMSetRenderTargets(1, &g_D3DBackBuffers[BackBufferIndex].m_uiCPUHandle, false, &mainDepth->m_uiCPUHandle);

    RenderImGUI();

    g_D3DBackBuffers[BackBufferIndex].TransisitonState(g_defaultCommandList.Get(), D3D12_RESOURCE_STATE_COMMON);
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

    g_D3DRayTracingScene.FlushRTScene();

    if (!SUCCEEDED(D3DDevice::s_swapchain->Present(1, 0)))
    {
        OutputDebugStringA("Error : Swapchain Present \n");
        assert(0);
    }

    BackBufferIndex = (BackBufferIndex + 1) % 2;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_SRVDescriptorHeap;

int main()
{
    g_Camera.transform.position.x = 0;
    g_Camera.transform.position.y = 0;
    g_Camera.transform.position.z = -10;

    g_Camera.transform.rotation.rotationMatrix = DirectX::XMMatrixIdentity();

    g_ScreenResolution.width = 1280;
    g_ScreenResolution.height = 720;
    
    g_clockBegin = std::chrono::steady_clock::now();

    D3DDevice::InitializeDevice();
    initD3DCommandsStructs();

    createD3DWindow(g_ScreenResolution.width, g_ScreenResolution.height);

    //initD3DSwapchain(g_ScreenResolution.width, g_ScreenResolution.height);
    D3DDevice::InitializeSwapchain(g_ScreenResolution.width, g_ScreenResolution.height, g_windowHandle, g_commandQueue.Get());

    initD3DRenderTargets();

    g_D3DBufferManager.Initialize(D3DDevice::s_device.Get(), 20000);
    g_D3DBufferManager.SetDebugName(L"SRV Descriptor Heap");
    
    g_GameScene.Initialize();

    g_D3DRayTracingScene.Initialize(D3DDevice::s_device.Get());

    g_D3DBufferManager.InitializeConstantBuffer(&g_GameScene.m_pSceneConstantBuffer, sizeof(GameSceneData));
    g_GameScene.m_pSceneConstantBuffer.SetDebugName(L"Scene Constant Buffer");

    InitializeImGUI();

    g_GameScene.m_oSceneData.oViewProjMatrix = DirectX::XMMatrixIdentity();
    g_GameScene.m_oSceneData.oDirectionalLight.color.r = 1;
    g_GameScene.m_oSceneData.oDirectionalLight.color.g = 1;
    g_GameScene.m_oSceneData.oDirectionalLight.color.b = 0.7f;
    g_GameScene.m_oSceneData.oDirectionalLight.angle.x = -1; // TODO : Normalize the angle
    g_GameScene.m_oSceneData.oDirectionalLight.angle.y = -1; // TODO : Normalize the angle
    g_GameScene.m_oSceneData.oDirectionalLight.angle.z = -1; // TODO : Normalize the angle

    GamePointLight pl0;
    GamePointLight pl1;
    pl0.color.r = 10;
    pl0.color.g = 0;
    pl0.color.b = 0;
    pl0.radius = 25.0f;
    pl0.position.x = 5.0f;
    pl0.position.y = 15.0f;
    pl0.position.z = 2.0f;

    pl1.color.r = 0;
    pl1.color.g = 0;
    pl1.color.b = 1;
    pl1.radius = 25.0f;
    pl1.position.x = -10.0f;
    pl1.position.y = 7.0f;
    pl1.position.z = 0.0f;

    g_GameScene.AddPointLight(pl0);
    g_GameScene.AddPointLight(pl1);
    g_GameScene.UploadPointLightsToGPU();

    D3DMesh oGroundMesh;
    D3DMesh oMesh;
    D3DMesh oMesh2;
    D3DMesh oMesh3;
    D3DMesh oMesh4;

    oGroundMesh.Initialize("ground", D3DDevice::s_device.Get(), true);
    oMesh.Initialize("monkey", D3DDevice::s_device.Get(), true);
    oMesh2.Initialize("monkey2", D3DDevice::s_device.Get(), true);
    oMesh3.Initialize("monkeysmooth", D3DDevice::s_device.Get(), true);
    oMesh4.Initialize("spheresmooth", D3DDevice::s_device.Get(), true);

    D3DMesh::s_MeshList.push_back(oGroundMesh);
    D3DMesh::s_MeshList.push_back(oMesh);
    D3DMesh::s_MeshList.push_back(oMesh2);
    D3DMesh::s_MeshList.push_back(oMesh3);
    D3DMesh::s_MeshList.push_back(oMesh4);

    test_texture = new D3DTexture;
    test_depth = new D3DTexture;
    mainRT = new D3DRenderTarget;
    mainDepth = new D3DDepthBuffer;

    g_D3DBufferManager.InitializeTexture(test_texture, g_ScreenResolution.width, g_ScreenResolution.height, BACK_BUFFER_FORMAT);
    g_D3DBufferManager.InitializeTexture(test_depth, g_ScreenResolution.width, g_ScreenResolution.height, DEPTH_BUFFER_FORMAT, true);

    g_D3DRenderTargetManager.InitializeRenderTargetFromTexture(mainRT, test_texture);
    g_D3DRenderTargetManager.InitializeDepthBufferFromTexture(mainDepth, test_depth);

    mainRT->SetDebugName(L"Main Render Target");
    mainDepth->SetDebugName(L"Main Depth");

    MSG message = {};

    while (g_bIsRunning)
    {
        DrawImGUI();
        
        // Game Loop
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

        // Window Events
        while (PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                g_bIsRunning = false;
            }

            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }

    return 0;
}
