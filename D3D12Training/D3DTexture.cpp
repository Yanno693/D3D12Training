#include "D3DTexture.h"

UINT D3DTexture::GetWidth() const
{
    return m_uiWidth;
}

UINT D3DTexture::GetHeight() const
{
    return m_uiHeight;
}

UINT D3DTexture::GetMipCount() const
{
	return m_uiMipCount;
}

UINT D3DTexture::GetRowPitch() const
{
	return m_uiRowPitch;
}

UINT64 D3DTexture::GetResourceSize() const
{
	return m_uiResourceSize;
}

DXGI_FORMAT D3DTexture::GetFormat() const
{
	return m_eFormat;
}

void D3DTexture::TransitionState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState)
{
	assert(m_pResource != nullptr);

	if (m_eCurrentState != a_targetResourceState)
	{
		D3D12_RESOURCE_BARRIER transitionBarrier = {};
		transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transitionBarrier.UAV.pResource = NULL;
		transitionBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		transitionBarrier.Transition.pResource = m_pResource.Get();
		transitionBarrier.Transition.StateBefore = m_eCurrentState;
		transitionBarrier.Transition.StateAfter = a_targetResourceState;

		D3D12_RESOURCE_BARRIER transitionBarriers[1] = { transitionBarrier };

		a_commandList->ResourceBarrier(1, transitionBarriers);

		m_eCurrentState = a_targetResourceState;
	}
}

void D3DTexture::WaitForUAV(ID3D12GraphicsCommandList* a_commandList)
{
	assert(m_pResource != nullptr);

	D3D12_RESOURCE_BARRIER transitionBarrier = {};
	transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transitionBarrier.UAV.pResource = m_pResource.Get();

	D3D12_RESOURCE_BARRIER transitionBarriers[1] = { transitionBarrier };

	a_commandList->ResourceBarrier(1, transitionBarriers);

}

void D3DTexture::SetDebugName(LPCWSTR a_sName)
{
    assert(m_pResource != nullptr);
    m_pResource->SetName(a_sName);
}
