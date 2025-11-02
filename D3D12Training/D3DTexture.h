#pragma once

#include "D3DIncludes.h"

class D3DTexture
{
	friend class D3DBufferManager;

private:
	ID3D12Device* m_pDevice = nullptr; // Pointer to the device that created the ressource
	UINT64 m_uiResourceSize = 0; // Resource Size in Byte
	D3D12_RESOURCE_STATES m_eCurrentState = D3D12_RESOURCE_STATE_COMMON;
	UINT m_uiWidth = 0;
	UINT m_uiHeight = 0;
	UINT m_uiMipCount = 0;
	DXGI_FORMAT m_eFormat = DXGI_FORMAT_UNKNOWN;

	char* m_pUploadData = nullptr; // Texture data to upload before the texture is raedy to use

public:
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_oUAVView;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_oSRVView;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
	D3D12_GPU_DESCRIPTOR_HANDLE m_eUAVGPUHandle; // UAV GPU Handle, sort of "address" in the descriptor heap
	D3D12_GPU_DESCRIPTOR_HANDLE m_eSRVGPUHandle; // SRV GPU Handle, sort of "address" in the descriptor heap

	//void Initialize(ID3D12Device* a_pDevice, UINT a_uiSizeInBytes);
	void TransitionState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState);
	void WaitForUAV(ID3D12GraphicsCommandList* a_commandList);
	//void WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset = 0);
	void SetDebugName(LPCWSTR a_sName);

	UINT GetWidth() const;
	UINT GetHeight() const;
	UINT GetMipCount() const;
	UINT64 GetResourceSize() const;
	DXGI_FORMAT GetFormat() const;
};