#pragma once

#include "D3DIncludes.h"

class D3DDepthBuffer
{
	friend class D3DRenderTargetManager;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCPUHandle = { 0 };
	D3D12_RESOURCE_STATES m_eCurrentState = D3D12_RESOURCE_STATE_COMMON;

	void TransisitonState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState);

	void SetDebugName(LPCWSTR a_sName);
};