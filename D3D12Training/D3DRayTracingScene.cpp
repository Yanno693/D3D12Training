#include "D3DRayTracingScene.h"
#include "D3DDevice.h"

void D3DRayTracingScene::Initialize(ID3D12Device5* a_pDevice)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	if (!m_bInitialized)
	{
		m_pDevice = a_pDevice;
		m_bInitialized = true;

		g_D3DBufferManager.RequestDescriptor(m_uiBVH_CPUHandle, m_uiBVH_GPUHandle);

		m_oSceneRGShader = static_cast<D3DRayGenerationShader*>(g_D3DShaderManager.RequestRTShaderV2("default", RAYGEN));
	}
}

void D3DRayTracingScene::setRenderTarget(D3DTexture* a_pTexture)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	m_pRenderTarget = a_pTexture;
}

void D3DRayTracingScene::SubmitForDraw(D3DMesh* a_pMesh)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	assert(m_bInitialized);
	assert(m_pDevice);
	
	m_apCurrentSceneMesh.push_back(a_pMesh);
}

void D3DRayTracingScene::DrawScene(ID3D12GraphicsCommandList4* a_pCommandList)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;

	assert(m_bInitialized);
	assert(m_pDevice);
	assert(m_pRenderTarget);

	for (UINT i = 0; i < m_apCurrentSceneMesh.size(); i++)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC oMeshBuildDesc = {};
		oMeshBuildDesc.DestAccelerationStructureData = m_apCurrentSceneMesh[i]->m_oBVH.m_pResource.Get()->GetGPUVirtualAddress();
		oMeshBuildDesc.ScratchAccelerationStructureData = m_apCurrentSceneMesh[i]->m_oBVHScratch.m_pResource.Get()->GetGPUVirtualAddress();
		oMeshBuildDesc.Inputs = m_apCurrentSceneMesh[i]->m_oBVHInput;

		a_pCommandList->BuildRaytracingAccelerationStructure(&oMeshBuildDesc, 0, nullptr);
	}

	CreateBVH(a_pCommandList);

	a_pCommandList->SetPipelineState1(m_apCurrentSceneMesh[0]->m_pRayTracingPso.Get());
	a_pCommandList->SetComputeRootSignature(m_apCurrentSceneMesh[0]->m_pRayTracingRootSignature.Get());

	ID3D12DescriptorHeap* heaps[1];
	heaps[0] = g_D3DBufferManager.GetHeap();

	a_pCommandList->SetDescriptorHeaps(1, heaps);
	a_pCommandList->SetComputeRootDescriptorTable(0, m_pRenderTarget->m_eUAVGPUHandle);
	a_pCommandList->SetComputeRootShaderResourceView(1, m_oBVH.m_pResource->GetGPUVirtualAddress());
	a_pCommandList->SetComputeRootConstantBufferView(2, g_SceneConstantBuffer.m_pResource.Get()->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC oRayDispatch = {};
	oRayDispatch.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.RayGenerationShaderRecord.StartAddress = m_apCurrentSceneMesh[0]->m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress();
	oRayDispatch.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.MissShaderTable.StartAddress = m_apCurrentSceneMesh[0]->m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.HitGroupTable.StartAddress = m_apCurrentSceneMesh[0]->m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.Width = m_pRenderTarget->GetWidth();
	oRayDispatch.Height = m_pRenderTarget->GetHeight();
	oRayDispatch.Depth = 1;

	a_pCommandList->DispatchRays(&oRayDispatch);
}

void D3DRayTracingScene::CreateBVH(ID3D12GraphicsCommandList4* a_pCommandList)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;

	g_D3DBufferManager.InitializeGenericBuffer(&m_oInstanceUpdateBuffer, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_apCurrentSceneMesh.size());
	m_oInstanceUpdateBuffer.SetDebugName(L"RT Scene Instance Buffer Update");

	g_D3DBufferManager.InitializeGenericBuffer(&m_oSceneShaderIDBuffer, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * g_D3DShaderManager.GetRTShadersCount());

	D3D12_RAYTRACING_INSTANCE_DESC* pInstancesData = (D3D12_RAYTRACING_INSTANCE_DESC*)malloc(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_apCurrentSceneMesh.size());
	assert(pInstancesData != nullptr);

	for (UINT i = 0; i < m_apCurrentSceneMesh.size(); i++)
	{
		pInstancesData[i] = {};
		pInstancesData[i].InstanceID = i;
		pInstancesData[i].InstanceMask = 1;
		pInstancesData[i].AccelerationStructure = m_apCurrentSceneMesh[i]->m_oBVH.m_pResource.Get()->GetGPUVirtualAddress();

		DirectX::XMMATRIX oTransform = DirectX::XMMatrixIdentity(); // TODO : Pass transform from the object
		
		//oTransform *= DirectX::XMMatrixTranslation(0, 1, 0);
		DirectX::XMStoreFloat3x4((DirectX::XMFLOAT3X4*)&pInstancesData[i].Transform, oTransform);

		// Todo : Set matrix for objects ! it's done here it seems !

	}
	m_oInstanceUpdateBuffer.WriteData(pInstancesData, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_apCurrentSceneMesh.size());
	free(pInstancesData);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS oBVHInput = {};
	oBVHInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	oBVHInput.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	oBVHInput.NumDescs = m_apCurrentSceneMesh.size();
	oBVHInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	oBVHInput.InstanceDescs = m_oInstanceUpdateBuffer.m_pResource.Get()->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO oBVHPreBuildInfo = {};
	m_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&oBVHInput, &oBVHPreBuildInfo);

	g_D3DBufferManager.InitializeGenericBuffer(&m_oBVH, oBVHPreBuildInfo.ResultDataMaxSizeInBytes, true, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	m_oBVH.SetDebugName(L"Scene BVH Buffer");
	g_D3DBufferManager.InitializeGenericBuffer(&m_oBVHScratch, oBVHPreBuildInfo.ScratchDataSizeInBytes, true);
	m_oBVHScratch.SetDebugName(L"Scene BVH Scratch Buffer");
	m_oBVHScratch.TransitionState(a_pCommandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC oBVHBuildDesc = {};
	oBVHBuildDesc.DestAccelerationStructureData = m_oBVH.m_pResource.Get()->GetGPUVirtualAddress();
	oBVHBuildDesc.Inputs = oBVHInput;
	oBVHBuildDesc.ScratchAccelerationStructureData = m_oBVHScratch.m_pResource.Get()->GetGPUVirtualAddress();

	a_pCommandList->BuildRaytracingAccelerationStructure(&oBVHBuildDesc, 0, nullptr);

	m_oBVHScratch.WaitForUAV(a_pCommandList);
};

void D3DRayTracingScene::ReleaseBVH()
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	/*
	m_oInstanceUpdateBuffer.m_pResource->Release();
	m_oBVH.m_pResource->Release();
	m_oBVHScratch.m_pResource->Release();
	*/

	// Huh ? Why does THIS works ?
	m_oInstanceUpdateBuffer.m_pResource.ReleaseAndGetAddressOf();
	m_oBVH.m_pResource.ReleaseAndGetAddressOf();
	m_oBVHScratch.m_pResource.ReleaseAndGetAddressOf();
	m_oSceneShaderIDBuffer.m_pResource.ReleaseAndGetAddressOf();

	/*
	m_oInstanceUpdateBuffer.m_pResource.Reset();
	m_oBVH.m_pResource.Reset();
	m_oBVHScratch.m_pResource.Reset();
	*/
}

void D3DRayTracingScene::flushSceneMesh()
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	assert(m_bInitialized);
	assert(m_pDevice);
	m_apCurrentSceneMesh.clear();
	ReleaseBVH();
}

D3DRayTracingScene g_D3DRayTracingScene;
