#pragma once

#include "D3DIncludes.h"
#include "D3DConstantBuffer.h"
#include "D3DGenericBuffer.h"
#include "D3DVertexBuffer.h"
#include "D3DIndexBuffer.h"
#include "D3DTexture.h"

#include <unordered_map>
#include <map>
#include <queue>
#include <deque>
#include <utility>

class D3DBufferManager
{
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeap = nullptr; // Descriptor Heap for the manager
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCurrentDecriptorOffset = {}; // Current offet in heap memory
	D3D12_GPU_DESCRIPTOR_HANDLE m_uiCurrentGPUDecriptorOffset = {}; // Current GPU offet in heap memory (with the heap in Shader Visible)
	UINT m_uiSRVSizeInHeap = 0; // Size of one RTV in heap memory, used for updating the offset

	ID3D12Device* m_pDevice = nullptr; // A pointer to the device once initialized;

	void IncrementOffset();

	// Texture Uploading Stuff
	D3DTexture* LoadTextureFromDDSFile(std::string a_sPath, std::string a_sName);

	std::map<std::string, D3DTexture*> m_oLoadedTexture; // Texture loaded in GPU memory
	std::deque<std::pair<std::string, D3DTexture*>> m_oTextureToLoad; // Texture that need to be loaded to GPU memory

	D3DGenericBuffer* m_pUploadBuffer; // CPU Side buffer used for uploading textures
	UINT m_uiCurrentUploadBufferAllocation = 0; // Current occupation of the Upload Buffer, it must be smaller that the buffer size
	D3DTexture* GetLoadingTexture(std::string a_sName); // Get a texture already requested for loading

public:

	void Initialize(ID3D12Device* a_pDevice, int a_iNbDecriptors);

	void InitializeConstantBuffer(D3DConstantBuffer* a_pConstantBuffer, UINT const a_uiSizeInBytes, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeGenericBuffer(D3DGenericBuffer* a_pGenericBuffer, UINT const a_uiSizeInBytes, bool a_bUAV = false, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeIndexBuffer(D3DIndexBuffer* a_pIndexBuffer, UINT const a_uiSizeInBytes, bool a_bIsInt16 = false, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	void InitializeVertexBuffer(D3DVertexBuffer* a_pVertexBuffer, UINT const a_uiSizeInBytes, UINT const a_uiVertexSize, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	
	void InitializeTexture(D3DTexture* a_pTexture, UINT const a_uiWidth, UINT const a_uiHeight, DXGI_FORMAT const a_eTextureFormat, bool a_bIsDepth = false, D3D12_RESOURCE_STATES const a_eDefaultState = D3D12_RESOURCE_STATE_COMMON);
	D3DTexture* GetTexture(std::string a_sName); // Get a texture from the loaded textures
	D3DTexture* RequestTexture(std::string a_sName); // Get a texture from the loaded textures, and load it if not present
	D3DTexture* CreateTextureFromColor(std::string a_sName, float* a_pRGBAColor);
	D3DTexture* CreateTextureFromColor(std::string a_sName, float a_fR, float a_fG, float a_fB, float a_fA);

	void RequestDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& out_rCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE & out_rCGUHandle);

	void UploadTextures(ID3D12GraphicsCommandList* a_pCommandList);
	bool isUploadTextureQueueEmpty();

	ID3D12DescriptorHeap* GetHeap();

	void SetDebugName(LPCWSTR a_sName);
};

extern D3DBufferManager g_D3DBufferManager;
