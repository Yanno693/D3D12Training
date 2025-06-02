#pragma once
#include "D3DIncludes.h"
#include "D3DGenericBuffer.h"

class D3DVertexBuffer
{
	friend class D3DBufferManager;

private:
	ID3D12Device* m_pDevice = nullptr; // Pointer to the device that created the ressource
	UINT m_uiResourceSize = 0; // Resource Size in Byte
	D3D12_RESOURCE_STATES m_eCurrentState = D3D12_RESOURCE_STATE_COMMON;

public:
	D3D12_VERTEX_BUFFER_VIEW m_oView = {};
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource = nullptr;

	void Initialize(ID3D12Device* a_pDevice, UINT a_uiSizeInBytes);
	void TransitionState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState);
	void WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset = 0);
	void SetDebugName(LPCWSTR a_sName);
};

class D3DRayTracingVertexBuffer
{
public:
	D3D12_VERTEX_BUFFER_VIEW m_oData = {};
	D3DGenericBuffer m_oBuffer;

	void TransisitonState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState) { m_oBuffer.TransitionState(a_commandList, a_targetResourceState); }
	void SetDebugName(LPCWSTR a_sName) { m_oBuffer.SetDebugName(a_sName); }
	void WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset = 0) { m_oBuffer.WriteData(a_pData, a_uiSize, a_uiDstOffset); }
};
