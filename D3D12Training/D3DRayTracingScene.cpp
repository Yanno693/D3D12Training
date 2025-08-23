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

		LoadSceneDefaultShaders();
		CreateGlobalRayTracingRootSignature(a_pDevice);
	}
}

void D3DRayTracingScene::LoadSceneDefaultShaders()
{
	m_oSceneRGShader = static_cast<D3DRayGenerationShader*>(g_D3DShaderManager.RequestRTShaderV2("default", RAYGEN));
	m_oSceneMissShader = static_cast<D3DMissShader*>(g_D3DShaderManager.RequestRTShaderV2("default", MISS));
	m_oSceneMissShader2 = static_cast<D3DMissShader*>(g_D3DShaderManager.RequestRTShaderV2("default2", MISS));
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

	a_pCommandList->SetComputeRootSignature(m_pRayTracingRootSignature.Get());
	a_pCommandList->SetPipelineState1(m_pRayTracingPSO.Get());

	ID3D12DescriptorHeap* heaps[1];
	heaps[0] = g_D3DBufferManager.GetHeap();

	a_pCommandList->SetDescriptorHeaps(1, heaps);
	a_pCommandList->SetComputeRootDescriptorTable(0, m_pRenderTarget->m_eUAVGPUHandle);
	a_pCommandList->SetComputeRootShaderResourceView(1, m_oBVH.m_pResource->GetGPUVirtualAddress());
	a_pCommandList->SetComputeRootConstantBufferView(2, g_SceneConstantBuffer.m_pResource.Get()->GetGPUVirtualAddress());

	D3D12_DISPATCH_RAYS_DESC oRayDispatch = {};
	oRayDispatch.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.RayGenerationShaderRecord.StartAddress = m_oSceneShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress();

	oRayDispatch.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::MISS).size();
	oRayDispatch.MissShaderTable.StartAddress = m_oSceneShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	//oRayDispatch.MissShaderTable.StartAddress = m_oSceneShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	oRayDispatch.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES * g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::HIT).size();
//	oRayDispatch.HitGroupTable.StartAddress = m_oSceneShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + (1 + g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::MISS).size()) * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.HitGroupTable.StartAddress = m_oSceneShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.HitGroupTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	
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

	CreateGlobalRayTracingPSO(D3DDevice::s_Device.Get());
	CreateShaderIDBuffer();

	D3D12_RAYTRACING_INSTANCE_DESC* pInstancesData = (D3D12_RAYTRACING_INSTANCE_DESC*)malloc(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_apCurrentSceneMesh.size());
	assert(pInstancesData != nullptr);

	for (UINT i = 0; i < m_apCurrentSceneMesh.size(); i++)
	{
		pInstancesData[i] = {};
		pInstancesData[i].InstanceID = i;
		pInstancesData[i].InstanceMask = 0xFF;
		pInstancesData[i].AccelerationStructure = m_apCurrentSceneMesh[i]->m_oBVH.m_pResource.Get()->GetGPUVirtualAddress();

		pInstancesData[i].InstanceContributionToHitGroupIndex = i;

		GamePosition oMeshPosition = m_apCurrentSceneMesh[i]->GetPosition();
		DirectX::XMMatrixTranslation(oMeshPosition.x, oMeshPosition.y, oMeshPosition.z);
		DirectX::XMMATRIX oTransform = DirectX::XMMatrixTranslation(oMeshPosition.x, oMeshPosition.y, oMeshPosition.z);; // TODO : Pass transform from the object

		m_apCurrentSceneMesh[i]->GetPosition();

		DirectX::XMStoreFloat3x4((DirectX::XMFLOAT3X4*)&pInstancesData[i].Transform, oTransform);
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
}

void D3DRayTracingScene::CreateShaderIDBuffer()
{
	g_D3DBufferManager.InitializeGenericBuffer(&m_oSceneShaderIDBuffer, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * g_D3DShaderManager.GetRTShadersCount());
	m_oSceneShaderIDBuffer.SetDebugName(L"RT Scene Shader ID Buffer"); // You'll need to create the RT PSO to fill the shader id buffer of the identifiers

	auto& oRGShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::RAYGEN);
	auto& oMissShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::MISS);
	auto& oHitShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::HIT);

	const UINT c_uiShaderCount = g_D3DShaderManager.GetRTShadersCount();

	char* apShaderIDData = (char*)malloc(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * c_uiShaderCount);
	assert(apShaderIDData != nullptr);
	ZeroMemory(apShaderIDData, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * c_uiShaderCount);

	ID3D12StateObjectProperties* oRTPSOProperties;
	if (!SUCCEEDED(m_pRayTracingPSO.Get()->QueryInterface(&oRTPSOProperties)))
	{
		OutputDebugStringA("Error : Query Ray Tracing PSO\n");
		assert(0);
	}

	// 1. Ray Generation shader, only 32 btis but the next Miss Shaders will need to start 64 bit away from the buffer start
	void* pShaderID = oRTPSOProperties->GetShaderIdentifier(L"default_raygen");
	memcpy(apShaderIDData, pShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	// 2. Miss shader, only two for now, so 64 bit aligned, 128 bits for the Hit shaders
	for (const auto& roMissShader : oMissShaderSet)
	{
		std::wstring szWideShaderIdentifier(roMissShader.second->m_szShaderIdentifier.begin(), roMissShader.second->m_szShaderIdentifier.end());
		pShaderID = oRTPSOProperties->GetShaderIdentifier(szWideShaderIdentifier.c_str());

		const D3DMissShader* pMissShader = (D3DMissShader*)roMissShader.second;
		memcpy(apShaderIDData + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT + pMissShader->m_uiShaderTableIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, pShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	// 3. Hit shader
	for (const auto& roHitShader : oHitShaderSet)
	{
		std::wstring szWideShaderIdentifier(roHitShader.second->m_szShaderIdentifier.begin(), roHitShader.second->m_szShaderIdentifier.end());
		szWideShaderIdentifier += L"group";
		pShaderID = oRTPSOProperties->GetShaderIdentifier(szWideShaderIdentifier.c_str());

		const D3DHitShader* pHitShader = (D3DHitShader*)roHitShader.second;
		//memcpy(apShaderIDData + (1 + oMissShaderSet.size() + pHitShader->m_uiShaderTableIndex) * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, pShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		memcpy(apShaderIDData + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT + pHitShader->m_uiShaderTableIndex * D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES , pShaderID, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	m_oSceneShaderIDBuffer.WriteData(apShaderIDData, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * c_uiShaderCount);

	oRTPSOProperties->Release();
	free(apShaderIDData);
}

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

void D3DRayTracingScene::CreateGlobalRayTracingRootSignature(ID3D12Device5* a_pDevice)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;

	if (m_pRayTracingRootSignature.Get() != nullptr)
		return;

	// It seems UAV have to be set in Descriptor Tables always ? 
	D3D12_DESCRIPTOR_RANGE oUAVRange = {};
	oUAVRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	oUAVRange.NumDescriptors = 1;

	D3D12_ROOT_PARAMETER oUAVRootParameter = {};
	oUAVRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	oUAVRootParameter.DescriptorTable.NumDescriptorRanges = 1;
	oUAVRootParameter.DescriptorTable.pDescriptorRanges = &oUAVRange;

	D3D12_ROOT_PARAMETER oBVHRootParameter = {};
	oBVHRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	oBVHRootParameter.Descriptor.RegisterSpace = 0;
	oBVHRootParameter.Descriptor.ShaderRegister = 0;

	D3D12_ROOT_PARAMETER oSceneConstantsParameter = {};
	oSceneConstantsParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	oSceneConstantsParameter.Descriptor.RegisterSpace = 0;
	oSceneConstantsParameter.Descriptor.ShaderRegister = 0;

	D3D12_ROOT_PARAMETER pRayTracingRootParameters[] = { oUAVRootParameter, oBVHRootParameter, oSceneConstantsParameter };

	Microsoft::WRL::ComPtr<ID3DBlob> rsBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(pRayTracingRootParameters);
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = NULL;
	rsDesc.pParameters = pRayTracingRootParameters;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
	if (errorBlob != nullptr)
	{
		void* d = errorBlob->GetBufferPointer(); // TODO : Display error message
		int a = 1;
		assert(0);
	}

	if (!SUCCEEDED(a_pDevice->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRayTracingRootSignature))))
	{
		OutputDebugStringA("Error : Create Root signature\n");
		assert(0);
	}
	m_pRayTracingRootSignature.Get()->SetName(L"Scene RT Root Signature");
}

void D3DRayTracingScene::CreateGlobalRayTracingPSO(ID3D12Device5* a_pDevice)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;

	if (!g_D3DShaderManager.IsRTPSODirty())
		return;

	if (m_pRayTracingPSO.Get() != nullptr)
	{
		m_pRayTracingPSO.ReleaseAndGetAddressOf();
	}

	auto& oRGShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::RAYGEN);
	auto& oMissShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::MISS);
	auto& oHitShaderSet = g_D3DShaderManager.GetRTShaderSet(D3D_RT_SHADER_TYPE::HIT);

	// 1 for Ray Gen Shader
	// n for Miss Shaders
	// n for Hit Shaders
	// n for Hit Group (for each Hit Shader)
	// 1 for Shader Config
	// 1 for Root Signature
	// 1 for RT Pipeline Config
	const UINT c_uiSubobjectCount = 4 + oHitShaderSet.size() * 2 + oMissShaderSet.size();
	UINT uiSubobjectCounter = 0;
	D3D12_STATE_SUBOBJECT* aSubobject = new D3D12_STATE_SUBOBJECT[c_uiSubobjectCount];

	std::vector<D3D12_DXIL_LIBRARY_DESC> vDXILLibs;
	std::vector<D3D12_HIT_GROUP_DESC> vHitGroups;
	std::vector<std::wstring> vHitShaderName;
	std::vector<std::wstring> vHitGroupName;
	vDXILLibs.reserve(c_uiSubobjectCount);
	vHitGroups.reserve(c_uiSubobjectCount);
	vHitShaderName.reserve(c_uiSubobjectCount);
	vHitGroupName.reserve(c_uiSubobjectCount);
	UINT uiObjectListCounter = 0;

	// 1. Ray Generation Shader, there's always only one (for now ?)
	D3DRayGenerationShader* oSceneRGShader = (D3DRayGenerationShader*)oRGShaderSet["default"];

	D3D12_SHADER_BYTECODE oRGByteCode;
	oRGByteCode.pShaderBytecode = oSceneRGShader->m_pByteCode;
	oRGByteCode.BytecodeLength = oSceneRGShader->m_uiByteCodeSize;

	D3D12_DXIL_LIBRARY_DESC oRGDXILLib = {};
	oRGDXILLib.DXILLibrary = oRGByteCode;

	D3D12_STATE_SUBOBJECT oRGLibSubobject = {};
	oRGLibSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	oRGLibSubobject.pDesc = &oRGDXILLib;


	aSubobject[uiSubobjectCounter] = oRGLibSubobject;
	uiSubobjectCounter++;

	// 2. Miss Shaders
	for (const auto& roMissShader : oMissShaderSet)
	{
		D3D12_SHADER_BYTECODE oMissByteCode;
		oMissByteCode.pShaderBytecode = roMissShader.second->m_pByteCode;
		oMissByteCode.BytecodeLength = roMissShader.second->m_uiByteCodeSize;

		D3D12_DXIL_LIBRARY_DESC oMissDXILLib = {};
		oMissDXILLib.DXILLibrary = oMissByteCode;

		vDXILLibs.emplace_back(oMissDXILLib);

		D3D12_STATE_SUBOBJECT oMissLibSubobject = {};
		oMissLibSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		//oMissLibSubobject.pDesc = &oMissDXILLib;
		oMissLibSubobject.pDesc = &vDXILLibs[uiObjectListCounter];

		aSubobject[uiSubobjectCounter] = oMissLibSubobject;
		uiSubobjectCounter++;
		uiObjectListCounter++;
	}

	// 3. Hit Shaders + Hit Group
	for (const auto& roHitShader : oHitShaderSet)
	{
		std::wstring szHitShaderName(roHitShader.second->m_szShaderIdentifier.begin(), roHitShader.second->m_szShaderIdentifier.end());
		std::wstring szHitGroupName(roHitShader.second->m_szShaderIdentifier.begin(), roHitShader.second->m_szShaderIdentifier.end());
		szHitGroupName += L"group";

		vHitShaderName.emplace_back(szHitShaderName);
		vHitGroupName.emplace_back(szHitGroupName);
		
		D3D12_SHADER_BYTECODE oHitByteCode;
		oHitByteCode.pShaderBytecode = roHitShader.second->m_pByteCode;
		oHitByteCode.BytecodeLength = roHitShader.second->m_uiByteCodeSize;

		D3D12_DXIL_LIBRARY_DESC oHitDXILLib = {};
		oHitDXILLib.DXILLibrary = oHitByteCode;

		vDXILLibs.emplace_back(oHitDXILLib);

		D3D12_STATE_SUBOBJECT oHitLibSubobject = {};
		oHitLibSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		oHitLibSubobject.pDesc = &vDXILLibs[uiObjectListCounter];

		aSubobject[uiSubobjectCounter] = oHitLibSubobject;
		uiSubobjectCounter++;

		// 4. Hit Group
		D3D12_HIT_GROUP_DESC oHitGroup = {};
		oHitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		oHitGroup.HitGroupExport = vHitGroupName[uiObjectListCounter - oMissShaderSet.size()].c_str();
		oHitGroup.ClosestHitShaderImport = vHitShaderName[uiObjectListCounter - oMissShaderSet.size()].c_str(); // Not needed ?
		vHitGroups.emplace_back(oHitGroup);

		D3D12_STATE_SUBOBJECT oHitGroupSubobject = {};
		oHitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		oHitGroupSubobject.pDesc = &vHitGroups[uiObjectListCounter - oMissShaderSet.size()];

		aSubobject[uiSubobjectCounter] = oHitGroupSubobject;
		uiSubobjectCounter++;

		uiObjectListCounter++;
	}

	// 5. Shader config
	// This is the payload size, struct shared through shaders
	D3D12_RAYTRACING_SHADER_CONFIG oShaderConfig;
	oShaderConfig.MaxPayloadSizeInBytes = 4 * sizeof(float); // float4 for color, I guess ?
	oShaderConfig.MaxAttributeSizeInBytes = 2 * sizeof(float);
	D3D12_STATE_SUBOBJECT oShaderConfigSubobject = {};
	oShaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	oShaderConfigSubobject.pDesc = &oShaderConfig;
	aSubobject[uiSubobjectCounter] = oShaderConfigSubobject;
	uiSubobjectCounter++;

	// 6. Root Signature
	// This is the root signature, it's shared between the ray tracing shaders (same BVH, same UAV texture)
	D3D12_GLOBAL_ROOT_SIGNATURE oGlobalSig = {};
	oGlobalSig.pGlobalRootSignature = m_pRayTracingRootSignature.Get();
	D3D12_STATE_SUBOBJECT oGlobalRootSignatureSubobject = {};
	oGlobalRootSignatureSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	oGlobalRootSignatureSubobject.pDesc = &oGlobalSig;
	aSubobject[uiSubobjectCounter] = oGlobalRootSignatureSubobject;
	uiSubobjectCounter++;

	// 7. RT Pipeline Config
	// This holds the number of bounce it seems
	D3D12_RAYTRACING_PIPELINE_CONFIG oRTPipeline = {};
	oRTPipeline.MaxTraceRecursionDepth = 1;
	D3D12_STATE_SUBOBJECT oRTPipelineSubobject = {};
	oRTPipelineSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	oRTPipelineSubobject.pDesc = &oRTPipeline;
	aSubobject[uiSubobjectCounter] = oRTPipelineSubobject;
	uiSubobjectCounter++;

	D3D12_STATE_OBJECT_DESC oRayTracingPSODesc = {};
	oRayTracingPSODesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
	oRayTracingPSODesc.pSubobjects = aSubobject;
	oRayTracingPSODesc.NumSubobjects = c_uiSubobjectCount;

	if (!SUCCEEDED(a_pDevice->CreateStateObject(&oRayTracingPSODesc, IID_PPV_ARGS(&m_pRayTracingPSO))))
	{
		OutputDebugStringA("Error : Create Ray Tracing Root signature\n");
		assert(0);
	}
	m_pRayTracingPSO.Get()->SetName(L"Scene RT PSO");

	delete[] aSubobject;

	g_D3DShaderManager.SetRTPSOClean();
}

void D3DRayTracingScene::FlushRTScene()
{
	if (!D3DDevice::isRayTracingEnabled())
		return;
	
	assert(m_bInitialized);
	assert(m_pDevice);
	m_apCurrentSceneMesh.clear();
	ReleaseBVH();
}

ID3D12RootSignature* D3DRayTracingScene::GetGlobalRayTracingRootSignature() const
{
	return m_pRayTracingRootSignature.Get();
}

D3DRayTracingScene g_D3DRayTracingScene;
