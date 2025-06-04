#include "D3DDevice.h"

Microsoft::WRL::ComPtr<ID3D12Debug1>  D3DDevice::s_debugDevice;
Microsoft::WRL::ComPtr<IDXGIFactory4> D3DDevice::s_factory;
Microsoft::WRL::ComPtr<IDXGIAdapter1> D3DDevice::s_adapter;
Microsoft::WRL::ComPtr<ID3D12Device5>  D3DDevice::s_device;
bool D3DDevice::s_bIsRayTracingEnabled;

bool D3DDevice::isRayTracingEnabled()
{
    return s_bIsRayTracingEnabled;
}

void D3DDevice::InitializeDevice()
{
    // Set Debug Device (for ... debugging purpose obviously :))
    if (!SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&s_debugDevice))))
    {
        OutputDebugStringA("Error : debugeDevice \n");
        assert(0);
    }
    s_debugDevice->EnableDebugLayer();
    s_debugDevice->SetEnableGPUBasedValidation(true);

    if (!SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&s_factory))))
    {
        OutputDebugStringA("Error : CreateFactory \n");
        assert(0);
    }

    // Get the adapter with the most VRAM
    UINT itAdapter = 0;
    UINT finalAdapterID = 0;
    UINT64 maximumMemoryFound = 0;

    IDXGIAdapter1* pAdapter = nullptr;
    do {
        s_factory->EnumAdapters1(itAdapter, &pAdapter);
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        if (desc.DedicatedVideoMemory > maximumMemoryFound)
        {
            maximumMemoryFound = desc.DedicatedVideoMemory;
            finalAdapterID = itAdapter;
        }
        itAdapter++;
    } while (s_factory->EnumAdapters1(itAdapter, &pAdapter) != DXGI_ERROR_NOT_FOUND);
    s_factory->EnumAdapters1(finalAdapterID, &s_adapter);

    // Create device finally
    if (!SUCCEEDED(D3D12CreateDevice(s_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&s_device))))
    {
        if (!SUCCEEDED(D3D12CreateDevice(s_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&s_device))))
        {
            OutputDebugStringA("Error : Create Device \n");
            assert(0);
        }
        else
        {
            OutputDebugStringA("Create Device \n");
            s_bIsRayTracingEnabled = false;
        }
    }
    else
    {
        OutputDebugStringA("Create Device for RayTracing \n");
        s_bIsRayTracingEnabled = false;
    }

    // Initialize context

}

void D3DDevice::InitializeSwapchain(UINT const a_uiWidth, UINT const a_uiHeight)
{
    /*
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

    if (!SUCCEEDED(D3DDevice::s_factory->CreateSwapChainForHwnd(g_commandQueue.Get(), g_windowHandle, &desc, NULL, NULL, &g_swapchain)))
    {
        OutputDebugStringA("Error : Create SwapChain \n");
        assert(0);
    }
    */
}
