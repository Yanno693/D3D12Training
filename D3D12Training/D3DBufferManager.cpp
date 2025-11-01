#include "D3DBufferManager.h"

#include "DDS.h"
#include <fstream>

void D3DBufferManager::Initialize(ID3D12Device* a_pDevice, int a_iNbDecriptors)
{
    assert(m_pDevice == nullptr);
    m_pDevice = a_pDevice;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = a_iNbDecriptors;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.NodeMask = 0;

    if (!SUCCEEDED(a_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeap))))
    {
        OutputDebugStringA("Error : D3D Render Target Manager Initialization.");
        assert(0);
    }

    m_uiCurrentDecriptorOffset = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_uiCurrentGPUDecriptorOffset = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    m_uiSRVSizeInHeap = a_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void D3DBufferManager::InitializeConstantBuffer(D3DConstantBuffer* a_pConstantBuffer, UINT const a_uiSizeInBytes, D3D12_RESOURCE_STATES const a_eDefaultState)
{
    UINT ui256MultipleSize = max(256, 256 * ((a_uiSizeInBytes / 256) + 1)); // TODO : COrrect when it's already multiple of 256
    
    assert(m_pDevice != nullptr);

    D3D12_HEAP_DESC heapDesc = {};
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.SizeInBytes = ui256MultipleSize;
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
    resourceDesc.Width = ui256MultipleSize;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, a_eDefaultState, NULL, IID_PPV_ARGS(&a_pConstantBuffer->m_pResource))))
    {
        assert(0);
    }

    D3D12_CONSTANT_BUFFER_VIEW_DESC descCB = {};
    descCB.SizeInBytes = ui256MultipleSize;
    descCB.BufferLocation = a_pConstantBuffer->m_pResource->GetGPUVirtualAddress();

    m_pDevice->CreateConstantBufferView(&descCB, m_uiCurrentDecriptorOffset);
    a_pConstantBuffer->m_eGPUHandle = m_uiCurrentGPUDecriptorOffset;

    IncrementOffset();

    a_pConstantBuffer->m_pDevice = m_pDevice;
    a_pConstantBuffer->m_uiResourceSize = a_uiSizeInBytes;
    a_pConstantBuffer->m_eCurrentState = a_eDefaultState;
}

void D3DBufferManager::InitializeGenericBuffer(D3DGenericBuffer* a_pGenericBuffer, UINT const a_uiSizeInBytes, bool a_bUAV, D3D12_RESOURCE_STATES const a_eDefaultState)
{
    assert(m_pDevice != nullptr);

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
    resourceDesc.Flags = a_bUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : D3D12_RESOURCE_FLAG_NONE;

    // TODO : refactor other resource initializers with this method, this looks rad
    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
    assert(allocationInfo.SizeInBytes != UINT64_MAX);

    D3D12_HEAP_DESC heapDesc = {};
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
    heapDesc.Properties.Type = a_bUAV ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD;
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, a_eDefaultState, NULL, IID_PPV_ARGS(&a_pGenericBuffer->m_pResource))))
    {
        assert(0);
    }

    a_pGenericBuffer->m_eCurrentState = a_eDefaultState;
    a_pGenericBuffer->m_uiResourceSize = a_uiSizeInBytes;
}

void D3DBufferManager::InitializeIndexBuffer(D3DIndexBuffer* a_pIndexBuffer, UINT const a_uiSizeInBytes, bool a_bIsInt16, D3D12_RESOURCE_STATES const a_eDefaultState)
{
    assert(m_pDevice != nullptr);

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

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, a_eDefaultState, NULL, IID_PPV_ARGS(&a_pIndexBuffer->m_pResource))))
    {
        assert(0);
    }

    D3D12_INDEX_BUFFER_VIEW oView;
    oView.BufferLocation = a_pIndexBuffer->m_pResource->GetGPUVirtualAddress();
    oView.SizeInBytes = a_uiSizeInBytes;
    oView.Format = a_bIsInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

    IncrementOffset();

    a_pIndexBuffer->m_eCurrentState = a_eDefaultState;
    a_pIndexBuffer->m_oView = oView;
}

void D3DBufferManager::InitializeVertexBuffer(D3DVertexBuffer* a_pVertexBuffer, UINT const a_uiSizeInBytes, UINT const a_uiVertexSize, D3D12_RESOURCE_STATES const a_eDefaultState)
{
    assert(m_pDevice != nullptr);
    
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

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, a_eDefaultState, NULL, IID_PPV_ARGS(&a_pVertexBuffer->m_pResource))))
    {
        assert(0);
    }

    D3D12_VERTEX_BUFFER_VIEW oView;
    oView.BufferLocation = a_pVertexBuffer->m_pResource->GetGPUVirtualAddress();
    oView.SizeInBytes = a_uiSizeInBytes;
    oView.StrideInBytes = a_uiVertexSize;

    IncrementOffset();

    a_pVertexBuffer->m_eCurrentState = a_eDefaultState;
    a_pVertexBuffer->m_oView = oView;
}

DXGI_FORMAT DepthViewResourceFromFormat(DXGI_FORMAT a_eFormat)
{
    switch (a_eFormat)
    {
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
    case DXGI_FORMAT_D32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    case DXGI_FORMAT_D16_UNORM:
        return DXGI_FORMAT_R16_UNORM;
    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

void D3DBufferManager::InitializeTexture(D3DTexture* a_pTexture, UINT const a_uiWidth, UINT const a_uiHeight, DXGI_FORMAT const a_eTextureFormat, bool a_bIsDepth, D3D12_RESOURCE_STATES const a_eDefaultState)
{
    assert(m_pDevice != nullptr);

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Format = a_eTextureFormat;
    resourceDesc.MipLevels = 1;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Height = a_uiHeight;
    resourceDesc.Width = a_uiWidth;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    if (a_bIsDepth)
    {
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }
    else
    {
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }

    // TODO : refactor other resource initializers with this method, this looks rad
    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
    assert(allocationInfo.SizeInBytes != UINT64_MAX);

    D3D12_HEAP_DESC heapDesc = {};
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
    heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = a_eTextureFormat;
    clearValue.Color[0] = a_bIsDepth ? 1.0f : 0.0f;
    clearValue.Color[1] = 0;
    clearValue.Color[2] = 0;
    clearValue.Color[3] = 0;

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, a_eDefaultState, &clearValue, IID_PPV_ARGS(&a_pTexture->m_pResource))))
    {
        assert(0);
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
    D3D12_UNORDERED_ACCESS_VIEW_DESC descUAV = {};

    descSRV.Format = (a_bIsDepth ? DepthViewResourceFromFormat(a_eTextureFormat) : a_eTextureFormat);
    descSRV.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    descSRV.Texture2D.MostDetailedMip = 0;
    descSRV.Texture2D.MipLevels = -1;
    descSRV.Texture2D.PlaneSlice = 0;
    descSRV.Texture2D.ResourceMinLODClamp = 0;

    descUAV.Format = a_eTextureFormat;
    descUAV.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    descUAV.Texture2D.MipSlice = 0;
    descUAV.Texture2D.PlaneSlice = 0;

    m_pDevice->CreateShaderResourceView(a_pTexture->m_pResource.Get(), &descSRV, m_uiCurrentDecriptorOffset);

    a_pTexture->m_eSRVGPUHandle = m_uiCurrentGPUDecriptorOffset;
    a_pTexture->m_oSRVView = descSRV;

    IncrementOffset();

    if (!a_bIsDepth)
    {
        m_pDevice->CreateUnorderedAccessView(a_pTexture->m_pResource.Get(), nullptr, &descUAV, m_uiCurrentDecriptorOffset);

        a_pTexture->m_eUAVGPUHandle = m_uiCurrentGPUDecriptorOffset;
        a_pTexture->m_oUAVView = descUAV;

        IncrementOffset();
    }

    a_pTexture->m_uiResourceSize = allocationInfo.SizeInBytes;
    a_pTexture->m_eCurrentState = a_eDefaultState;

    a_pTexture->m_uiWidth = a_uiWidth;
    a_pTexture->m_uiHeight = a_uiHeight;
}

D3DTexture* D3DBufferManager::GetTexture(std::string a_sName)
{
    if (m_oLoadedTextureSet.find(a_sName) == m_oLoadedTextureSet.end())
        return nullptr;

    return m_oLoadedTextureSet[a_sName];
}

D3DTexture* D3DBufferManager::RequestTexture(std::string a_sName)
{
    D3DTexture* pTexture = GetTexture(a_sName);

    if (pTexture == nullptr)
    {
        std::string sPath = "./Textures/" + a_sName + ".dds";
        pTexture = LoadTextureFromDDSFile(sPath, a_sName);
    }

    return pTexture;
}

// Requestion reusable handles for external update (i'll recreate the scene BVH at each frame, but without using additionnal descriptors in the heap)
void D3DBufferManager::RequestDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE& out_rCPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE& out_rGPUHandle)
{
    out_rCPUHandle = m_uiCurrentDecriptorOffset;
    out_rGPUHandle = m_uiCurrentGPUDecriptorOffset;
    
    IncrementOffset();
}

ID3D12DescriptorHeap* D3DBufferManager::GetHeap()
{
    return m_pDescriptorHeap.Get();
}

void D3DBufferManager::IncrementOffset()
{
    assert(m_pDevice != nullptr);
    m_uiCurrentDecriptorOffset.ptr += m_uiSRVSizeInHeap;
    if (m_uiCurrentGPUDecriptorOffset.ptr != 0) // 0 mean we don't have a Shader Visible descriptor Heap
    {
        m_uiCurrentGPUDecriptorOffset.ptr += m_uiSRVSizeInHeap;
    }
}

DXGI_FORMAT GetDXGIFromDDSFormat(const DirectX::DDS_PIXELFORMAT& a_roPixelFormat)
{

    if ((a_roPixelFormat.flags & DDS_FOURCC) != 0) // BC Compression
    {
        if (a_roPixelFormat.fourCC == DirectX::DDSPF_DXT1.fourCC)
            return DXGI_FORMAT_BC1_UNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_DXT2.fourCC || a_roPixelFormat.fourCC == DirectX::DDSPF_DXT3.fourCC)
            return DXGI_FORMAT_BC2_UNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_DXT4.fourCC || a_roPixelFormat.fourCC == DirectX::DDSPF_DXT5.fourCC)
            return DXGI_FORMAT_BC3_UNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_BC4_UNORM.fourCC)
            return DXGI_FORMAT_BC4_UNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_BC4_SNORM.fourCC)
            return DXGI_FORMAT_BC4_SNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_BC5_UNORM.fourCC)
            return DXGI_FORMAT_BC5_UNORM;

        if (a_roPixelFormat.fourCC == DirectX::DDSPF_BC5_SNORM.fourCC)
            return DXGI_FORMAT_BC5_SNORM;
        
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3DTexture* D3DBufferManager::LoadTextureFromDDSFile(std::string a_sPath, std::string a_sName)
{
    // File reading
    D3DTexture* pTexture = new D3DTexture;

    std::ifstream oTextureStream(a_sPath, std::ios::binary);
    assert(oTextureStream);

    DirectX::DDS_HEADER oDDSTextureHeader;
    UINT uiMagicNumberCheck;
    
    oTextureStream.read((char*)&uiMagicNumberCheck, sizeof(UINT));
    assert(uiMagicNumberCheck == DirectX::DDS_MAGIC);
    oTextureStream.read((char*)&oDDSTextureHeader, sizeof(DirectX::DDS_HEADER));

    DirectX::DDS_PIXELFORMAT oPixelFormat = oDDSTextureHeader.ddspf;

    DXGI_FORMAT eFormat = GetDXGIFromDDSFormat(oPixelFormat);

    // Texture Resource initialization
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resourceDesc.Format = eFormat;
    resourceDesc.MipLevels = 1;
    resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resourceDesc.Height = oDDSTextureHeader.width;
    resourceDesc.Width = oDDSTextureHeader.height;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = m_pDevice->GetResourceAllocationInfo(0, 1, &resourceDesc);
    assert(allocationInfo.SizeInBytes != UINT64_MAX);

    D3D12_HEAP_DESC heapDesc = {};
    heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    heapDesc.Flags = D3D12_HEAP_FLAG_NONE;
    heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
    heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapDesc.Properties.CreationNodeMask = 0;
    heapDesc.Properties.VisibleNodeMask = 0;
    heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    if (!SUCCEEDED(m_pDevice->CreateCommittedResource(&heapDesc.Properties, heapDesc.Flags, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pTexture->m_pResource))))
    {
        assert(0);
    }

    pTexture->m_pUploadData = new char[allocationInfo.SizeInBytes];
    oTextureStream.read(pTexture->m_pUploadData, allocationInfo.SizeInBytes);

    oTextureStream.close();
    m_oLoadedTextureSet[a_sName] = pTexture;

    return pTexture;
}

void D3DBufferManager::SetDebugName(LPCWSTR a_sName)
{
    assert(m_pDescriptorHeap != nullptr);
    m_pDescriptorHeap->SetName(a_sName);
}

D3DBufferManager g_D3DBufferManager;