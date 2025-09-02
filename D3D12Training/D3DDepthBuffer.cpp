#include "D3DDepthBuffer.h"

void D3DDepthBuffer::TransisitonState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState)
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

D3DTexture* D3DDepthBuffer::GetD3DTexture()
{
	return m_pTexture;
}

void D3DDepthBuffer::SetDebugName(LPCWSTR a_sName)
{
	assert(m_pResource != nullptr);
	m_pResource->SetName(a_sName);
}
