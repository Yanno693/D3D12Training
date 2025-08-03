#pragma once

#include "D3DIncludes.h"

class D3DDevice
{
	static bool s_bIsRayTracingEnabled;
public :
	// Device and adapter
	static Microsoft::WRL::ComPtr<ID3D12Debug1> s_debugDevice;
	static Microsoft::WRL::ComPtr<IDXGIFactory4> s_factory;
	static Microsoft::WRL::ComPtr<IDXGIAdapter1> s_adapter;
	static Microsoft::WRL::ComPtr<ID3D12Device5> s_Device;

	static void InitializeDevice();
	static void InitializeSwapchain(UINT const a_uiWidth, UINT const a_uiHeight);
	static bool isRayTracingEnabled();
};