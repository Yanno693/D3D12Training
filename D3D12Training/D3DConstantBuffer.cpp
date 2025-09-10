#include "D3DConstantBuffer.h"

//D3DConstantBuffer g_SceneConstantBuffer;

// Do i do this here ? Or in the buffer manager ?
/*
void D3DConstantBuffer::Initialize(ID3D12Device* a_pDevice, UINT const a_uiSizeInBytes)
{
    m_pDevice = a_pDevice;

    D3D12_HEAP_DESC heapDesc = {};
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.SizeInBytes = a_uiSizeInBytes;
    heapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.MipLevels = 1;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Height = 1;
    resourceDesc.Width = a_uiSizeInBytes;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;

    if (!SUCCEEDED(a_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&m_pResource))))
    {
        assert(0);
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC descCB = {};
    descCB.SizeInBytes = a_uiSizeInBytes;
    descCB.BufferLocation = m_pResource->GetGPUVirtualAddress();

    //ID3D12GraphicsCommandList* a;
    //a->buffer

    m_eCurrentState = D3D12_RESOURCE_STATE_GENERIC_READ;

}
*/

void D3DConstantBuffer::TransisitonState(ID3D12GraphicsCommandList* a_commandList, D3D12_RESOURCE_STATES a_targetResourceState)
{
    assert(m_pDevice != nullptr);
    assert(m_uiResourceSize != 0);

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
    }

    m_eCurrentState = a_targetResourceState;
}

void D3DConstantBuffer::WriteData(void* a_pData, UINT a_uiSize, UINT a_uiDstOffset)
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

void D3DConstantBuffer::SetDebugName(LPCWSTR a_sName)
{
    assert(m_pResource != nullptr);
    m_pResource->SetName(a_sName);
}