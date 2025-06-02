#pragma once
#include "D3DIncludes.h"

class D3DGenericBuffer
{
	friend class D3DBufferManager;

private:
	ID3D12Device* m_pDevice = nullptr; // Pointer to the device that created the ressource
	UINT64 m_uiResourceSize = 0; // Resource Size in Byte
	D3D12_RESOURCE_STATES m_eCurrentState = D3D12_RESOURCE_STATE_COMMON;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;

	//void Initialize(ID3D12Device* a_pDevice, UINT a_uiSizeInBytes);
	void TransitionState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState);
	void WaitForUAV(ID3D12GraphicsCommandList* a_commandList);
	void WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset = 0);
	void SetDebugName(LPCWSTR a_sName);
};