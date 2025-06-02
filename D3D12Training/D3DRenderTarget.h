#pragma once

#include "D3DIncludes.h"
#include "D3DTexture.h"

class D3DRenderTarget
{
	friend class D3DRenderTargetManager;

	D3DTexture* m_pTexture = nullptr;

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiCPUHandle = {0};
	D3D12_RESOURCE_STATES m_eCurrentState = D3D12_RESOURCE_STATE_COMMON;

	void TransisitonState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState);
	D3DTexture* GetD3DTexture();
	void SetDebugName(LPCWSTR a_sName);
};

