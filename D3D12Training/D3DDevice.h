#pragma once

#include "D3DIncludes.h"

class D3DDevice
{
	static bool s_bIsRayTracingEnabled;
	static Microsoft::WRL::ComPtr<IDXGISwapChain1> s_swapchain1;

public :
	// Device and adapter
	static Microsoft::WRL::ComPtr<ID3D12Debug1> s_debugDevice;
	static Microsoft::WRL::ComPtr<IDXGIFactory4> s_factory;
	static Microsoft::WRL::ComPtr<IDXGIAdapter1> s_adapter;
	static Microsoft::WRL::ComPtr<ID3D12Device5> s_device;

	static Microsoft::WRL::ComPtr<IDXGISwapChain3> s_swapchain;

	static void InitializeDevice();
	static void InitializeSwapchain(UINT const a_uiWidth, UINT const a_uiHeight, HWND a_oWindowHandle, ID3D12CommandQueue* const a_pCommandQueue);
	static bool isRayTracingEnabled();
};