#pragma once

#include "D3DIncludes.h"
#include "D3DConstantBuffer.h"
#include "D3DGenericBuffer.h"
#include "D3DVertexBuffer.h"
#include "D3DIndexBuffer.h"
#include "D3DTexture.h"

class D3DBufferManager
{
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap; // Descriptor Heap for the manager
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCurrentDecriptorOffset; // Current offet in heap memory
	D3D12_GPU_DESCRIPTOR_HANDLE m_uiCurrentGPUDecriptorOffset; // Current GPU offet in heap memory (with the heap in Shader Visible)
	UINT m_uiSRVSizeInHeap = 0; // Size of one RTV in heap memory, used for updating the offset

	ID3D12Device* m_pDevice = nullptr; // A pointer tho the device once initialized;

	void IncrementOffset();

public:

	void Initialize(ID3D12Device* a_pDevice, int a_iNbDecriptors);

	void InitializeConstantBuffer(D3DConstantBuffer* a_pConstantBuffer, UINT const a_uiSizeInBytes, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeGenericBuffer(D3DGenericBuffer* a_pGenericBuffer, UINT const a_uiSizeInBytes, bool a_bUAV = false, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeIndexBuffer(D3DIndexBuffer* a_pIndexBuffer, UINT const a_uiSizeInBytes, bool a_bIsInt16 = false, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeVertexBuffer(D3DVertexBuffer* a_pVertexBuffer, UINT const a_uiSizeInBytes, UINT const a_uiVertexSize, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeTexture(D3DTexture* a_pTexture, UINT const a_uiWidth, UINT const a_uiHeight, DXGI_FORMAT const a_eTextureFormat, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);

	void RequestDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& out_rCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE & out_rCGUHandle);

	ID3D12DescriptorHeap* GetHeap();

	void SetDebugName(LPCWSTR a_sName);
};

extern D3DBufferManager g_D3DBufferManager;
