#include "D3DGenericBuffer.h"

void D3DGenericBuffer::TransitionState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState)
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

void D3DGenericBuffer::WaitForUAV(ID3D12GraphicsCommandList* a_commandList)
{
	assert(m_pResource != nullptr);

	D3D12_RESOURCE_BARRIER transitionBarrier = {};
	transitionBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	transitionBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	transitionBarrier.UAV.pResource = m_pResource.Get();

	D3D12_RESOURCE_BARRIER transitionBarriers[1] = { transitionBarrier };

	a_commandList->ResourceBarrier(1, transitionBarriers);

}

void D3DGenericBuffer::SetDebugName(LPCWSTR a_sName)
{
	assert(m_pResource != nullptr);
	m_pResource->SetName(a_sName);
}

void D3DGenericBuffer::WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset)
{
	D3D12_RANGE range;
	range.Begin = 0;
	range.End = 0;

	char* pBufferDst;

	if (!SUCCEEDED(m_pResource->Map(0, &range, (void**)&pBufferDst)))
	{
		assert(0);
	}

	memcpy(pBufferDst + a_uiDstOffset, a_pData, a_uiSize);
	m_pResource->Unmap(0, nullptr);
}