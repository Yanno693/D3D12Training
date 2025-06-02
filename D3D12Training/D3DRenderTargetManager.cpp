#include "D3DRenderTargetManager.h"

void D3DRenderTargetManager::IncrementRTVOffset()
{
    assert(m_pDevice != nullptr);
    m_uiCurrentRTVDecriptorOffset.ptr += m_uiRTVSizeInHeap;
}

void D3DRenderTargetManager::IncrementDSVOffset()
{
    assert(m_pDevice != nullptr);
    m_uiCurrentDSVDecriptorOffset.ptr += m_uiDSVSizeInHeap;
}

void D3DRenderTargetManager::Initialize(ID3D12Device* a_pDevice, int a_iNbDecriptors)
{
    m_pDevice = a_pDevice;
    
    D3D12_DESCRIPTOR_HEAP_DESC RTVdesc = {};
    RTVdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RTVdesc.NumDescriptors = a_iNbDecriptors;
    RTVdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    RTVdesc.NodeMask = 0;

    D3D12_DESCRIPTOR_HEAP_DESC DSVdesc = {};
    DSVdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DSVdesc.NumDescriptors = a_iNbDecriptors;
    DSVdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DSVdesc.NodeMask = 0;
	
    if (!SUCCEEDED(a_pDevice->CreateDescriptorHeap(&RTVdesc, IID_PPV_ARGS(&m_pRTVDescriptorHeap))))
    {
        OutputDebugStringA("Error : D3D Render Target Manager Initialization (RTV).");
        assert(0);
    }

    if (!SUCCEEDED(a_pDevice->CreateDescriptorHeap(&DSVdesc, IID_PPV_ARGS(&m_pDSVDescriptorHeap))))
    {
        OutputDebugStringA("Error : D3D Render Target Manager Initialization (DSV).");
        assert(0);
    }

    m_uiCurrentRTVDecriptorOffset = m_pRTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_uiCurrentDSVDecriptorOffset = m_pDSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_uiRTVSizeInHeap = a_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_uiDSVSizeInHeap = a_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void D3DRenderTargetManager::InitializeRenderTarget(D3DRenderTarget* a_pRenderTarget)
{
    assert(m_pDevice != nullptr);

    m_pDevice->CreateRenderTargetView(a_pRenderTarget->m_pResource.Get(), NULL, m_uiCurrentRTVDecriptorOffset);
    a_pRenderTarget->m_uiCPUHandle = m_uiCurrentRTVDecriptorOffset;

    IncrementRTVOffset();
}

void D3DRenderTargetManager::InitializeRenderTargetFromTexture(D3DRenderTarget* a_pRenderTarget, D3DTexture* a_pTexture)
{
    assert(m_pDevice != nullptr);

    m_pDevice->CreateRenderTargetView(a_pTexture->m_pResource.Get(), NULL, m_uiCurrentRTVDecriptorOffset);
    a_pRenderTarget->m_uiCPUHandle = m_uiCurrentRTVDecriptorOffset;
    a_pRenderTarget->m_pResource = a_pTexture->m_pResource;
    a_pRenderTarget->m_pTexture = a_pTexture;
    
    IncrementRTVOffset();
}

void D3DRenderTargetManager::InitializeDepthBuffer(D3DDepthBuffer* a_pDepthBuffer)
{
    assert(m_pDevice != nullptr);

    m_pDevice->CreateDepthStencilView(a_pDepthBuffer->m_pResource.Get(), NULL, m_uiCurrentDSVDecriptorOffset);
    a_pDepthBuffer->m_uiCPUHandle = m_uiCurrentDSVDecriptorOffset;

    IncrementDSVOffset();
}

void D3DRenderTargetManager::SetDebugName(LPCWSTR a_sName)
{
    assert(m_pRTVDescriptorHeap != nullptr);
    m_pRTVDescriptorHeap->SetName(a_sName);
}