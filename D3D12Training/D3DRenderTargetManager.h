#pragma once

#include "D3DIncludes.h"
#include "D3DRenderTarget.h"
#include "D3DDepthBuffer.h"

class D3DRenderTargetManager
{
private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRTVDescriptorHeap; // RTV Descriptor Heap for the manager
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDSVDescriptorHeap; // DSV Descriptor Heap for the manager
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCurrentRTVDecriptorOffset = { 0 }; // Current offet in heap memory for Render Targets
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCurrentDSVDecriptorOffset = { 0 }; // Current offet in heap memory for Depth Buffers
	UINT m_uiRTVSizeInHeap = 0; // Size of one RTV in heap memory, used for updating the offset
	UINT m_uiDSVSizeInHeap = 0; // Size of one DSV in heap memory, used for updating the offset

	ID3D12Device* m_pDevice = nullptr; // A pointer to the device once initialized;

	void IncrementRTVOffset();
	void IncrementDSVOffset();
	
public:

	void Initialize(ID3D12Device* a_pDevice, int a_iNbDecriptors);
	void InitializeRenderTarget(D3DRenderTarget* a_pRenderTarget);
	void InitializeRenderTargetFromTexture(D3DRenderTarget* a_pRenderTarget, D3DTexture* a_pTexture);
	void InitializeDepthBuffer(D3DDepthBuffer* a_pDepthBuffer);

	void SetDebugName(LPCWSTR a_sName);
};