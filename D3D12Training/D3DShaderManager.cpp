#include "D3DShaderManager.h"

void D3DShaderManager::LoadShader(std::string const a_sPath)
{
	assert(m_oShaderSet.find(a_sPath) == m_oShaderSet.end());

	if (m_pUtils == nullptr)
	{
		if (!SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_pUtils))))
		{
			OutputDebugStringA("Error : DXC Utilitary for Shader Reflection\n");
			assert(0);
		}
	}
	
	std::string sVSPath = "./Shaders/" + a_sPath + "_vs.cso";
	std::string sPSPath = "./Shaders/" + a_sPath + "_ps.cso";
	std::string sReflectionPath = "./Shaders/" + a_sPath + "_refl.cso";

	// Open shader bytecode and reflection data
	HANDLE pVSFileHandle = CreateFileA(
		sVSPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pVSFileHandle != INVALID_HANDLE_VALUE);

	HANDLE pPSFileHandle = CreateFileA(
		sPSPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pPSFileHandle != INVALID_HANDLE_VALUE);

	HANDLE pReflectionFileHandle = CreateFileA(
		sReflectionPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pReflectionFileHandle != INVALID_HANDLE_VALUE);
	
	// Get bytecode length
	ULONG uiVSFileSize = GetFileSize(pVSFileHandle, nullptr);
	ULONG uiPSFileSize = GetFileSize(pPSFileHandle, nullptr);
	ULONG uiReflectionFileSize = GetFileSize(pReflectionFileHandle, nullptr);
	assert(uiVSFileSize != INVALID_FILE_SIZE);
	assert(uiPSFileSize != INVALID_FILE_SIZE);
	assert(uiReflectionFileSize != INVALID_FILE_SIZE);

	// Save bytecode in struct
	D3DShaderPair* pShaderPair = new D3DShaderPair;
	pShaderPair->m_pVS = new D3DVertexShader;
	pShaderPair->m_pPS = new D3DPixelShader;
	pShaderPair->m_pVS->m_pReflectionData = new char[uiReflectionFileSize];
	pShaderPair->m_pVS->m_uiReflectionDataSize = uiReflectionFileSize;
	pShaderPair->m_pVS->m_pByteCode = new char[uiVSFileSize];
	pShaderPair->m_pPS->m_pByteCode = new char[uiPSFileSize];
	pShaderPair->m_pVS->m_uiByteCodeSize = uiVSFileSize;
	pShaderPair->m_pPS->m_uiByteCodeSize = uiPSFileSize;

	BOOL bReflectionReadFileResult = ReadFile(pReflectionFileHandle, pShaderPair->m_pVS->m_pReflectionData, uiReflectionFileSize, NULL, NULL);
	BOOL bVSReadFileResult = ReadFile(pVSFileHandle, pShaderPair->m_pVS->m_pByteCode, uiVSFileSize, NULL, NULL);
	BOOL bPSReadFileResult = ReadFile(pPSFileHandle, pShaderPair->m_pPS->m_pByteCode, uiPSFileSize, NULL, NULL);
	assert(bReflectionReadFileResult != 0);
	assert(bVSReadFileResult != 0);
	assert(bPSReadFileResult != 0);

	// Files concants have been saved to buffers, let' close them
	CloseHandle(pReflectionFileHandle);
	CloseHandle(pVSFileHandle);
	CloseHandle(pPSFileHandle);


	// Let's create the Input Layout for the VS using shader reflection datas
	//Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection;
	DxcBuffer dcInfo;
	dcInfo.Encoding = DXC_CP_ACP;
	dcInfo.Ptr = pShaderPair->m_pVS->m_pReflectionData;
	dcInfo.Size = pShaderPair->m_pVS->m_uiReflectionDataSize;

	if (!SUCCEEDED(m_pUtils->CreateReflection(&dcInfo, IID_PPV_ARGS(&pShaderPair->m_pReflection))))
	{
		OutputDebugStringA("Error : Shader Reflection\n");
		assert(0);
	}

	D3D12_SHADER_DESC oShaderDesc;
	pShaderPair->m_pReflection->GetDesc(&oShaderDesc);
	std::vector<D3D12_INPUT_ELEMENT_DESC> vInputElements;

	DXGI_FORMAT a1DFormats[3] = {
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R32_FLOAT,
	};

	DXGI_FORMAT a2DFormats[3] = {
		DXGI_FORMAT_R32G32_UINT,
		DXGI_FORMAT_R32G32_SINT,
		DXGI_FORMAT_R32G32_FLOAT,
	};

	DXGI_FORMAT a3DFormats[3] = {
		DXGI_FORMAT_R32G32B32_UINT,
		DXGI_FORMAT_R32G32B32_SINT,
		DXGI_FORMAT_R32G32B32_FLOAT,
	};

	DXGI_FORMAT a4DFormats[3] = {
		DXGI_FORMAT_R32G32B32A32_UINT,
		DXGI_FORMAT_R32G32B32A32_SINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
	};

	for (UINT i = 0; i < oShaderDesc.InputParameters; i++)
	{
		D3D12_SIGNATURE_PARAMETER_DESC oSignatureElement;
		pShaderPair->m_pReflection->GetInputParameterDesc(i, &oSignatureElement);

		D3D12_INPUT_ELEMENT_DESC oInputElement = {};
		oInputElement.SemanticName = oSignatureElement.SemanticName;
		oInputElement.SemanticIndex = oSignatureElement.SemanticIndex;
		oInputElement.InputSlot = 0;
		oInputElement.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		oInputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		oInputElement.InstanceDataStepRate = 0;

		if (oSignatureElement.Mask == 1)
		{
			oInputElement.Format = a1DFormats[oSignatureElement.ComponentType - 1];
		}
		else if (oSignatureElement.Mask <= 3)
		{
			oInputElement.Format = a2DFormats[oSignatureElement.ComponentType - 1];

		}
		else if (oSignatureElement.Mask <= 7)
		{
			oInputElement.Format = a3DFormats[oSignatureElement.ComponentType - 1];
		}
		else if (oSignatureElement.Mask <= 15)
		{
			oInputElement.Format = a4DFormats[oSignatureElement.ComponentType - 1];
		}

		pShaderPair->m_vInputElements.push_back(oInputElement);
	}

	//pReflection->Release();

	m_oShaderSet.insert(std::make_pair(a_sPath, pShaderPair));
}

void D3DShaderManager::LoadRTShader(std::string const a_sPath)
{
	assert(m_oRTShaderSet.find(a_sPath) == m_oRTShaderSet.end());

	std::string sRGSPath = "./Shaders/" + a_sPath + "_rg.cso";
	std::string sHSPath = "./Shaders/" + a_sPath + "_ch.cso";
	std::string sMSPath = "./Shaders/" + a_sPath + "_miss.cso";

	// Open shader bytecode
	HANDLE pRGSFileHandle = CreateFileA(
		sRGSPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pRGSFileHandle != INVALID_HANDLE_VALUE);

	HANDLE pHSFileHandle = CreateFileA(
		sHSPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pHSFileHandle != INVALID_HANDLE_VALUE);

	HANDLE pMSFileHandle = CreateFileA(
		sMSPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pMSFileHandle != INVALID_HANDLE_VALUE);

	// Get bytecode length
	ULONG uiRGSFileSize = GetFileSize(pRGSFileHandle, nullptr);
	ULONG uiHSFileSize = GetFileSize(pHSFileHandle, nullptr);
	ULONG uiMSFileSize = GetFileSize(pMSFileHandle, nullptr);
	assert(uiRGSFileSize != INVALID_FILE_SIZE);
	assert(uiRGSFileSize != INVALID_FILE_SIZE);
	assert(uiMSFileSize != INVALID_FILE_SIZE);

	// Save bytecode in struct
	D3DRTShaderGroup* pRTShaderGroup = new D3DRTShaderGroup;
	pRTShaderGroup->m_pRGS = new D3DRayGenerationShader;
	pRTShaderGroup->m_pHS = new D3DHitShader;
	pRTShaderGroup->m_pMS = new D3DMissShader;

	pRTShaderGroup->m_pRGS->m_pByteCode = new char[uiRGSFileSize];
	pRTShaderGroup->m_pHS->m_pByteCode = new char[uiHSFileSize];
	pRTShaderGroup->m_pMS->m_pByteCode = new char[uiMSFileSize];
	pRTShaderGroup->m_pRGS->m_uiByteCodeSize = uiRGSFileSize;
	pRTShaderGroup->m_pHS->m_uiByteCodeSize = uiHSFileSize;
	pRTShaderGroup->m_pMS->m_uiByteCodeSize = uiMSFileSize;

	BOOL bRGSReadFileResult = ReadFile(pRGSFileHandle, pRTShaderGroup->m_pRGS->m_pByteCode, uiRGSFileSize, NULL, NULL);
	BOOL bHSReadFileResult = ReadFile(pHSFileHandle, pRTShaderGroup->m_pHS->m_pByteCode, uiHSFileSize, NULL, NULL);
	BOOL bMSReadFileResult = ReadFile(pMSFileHandle, pRTShaderGroup->m_pMS->m_pByteCode, uiMSFileSize, NULL, NULL);
	assert(bRGSReadFileResult != 0);
	assert(bHSReadFileResult != 0);
	assert(bMSReadFileResult != 0);

	// Files content have been saved to buffers, let's close them
	CloseHandle(pRGSFileHandle);
	CloseHandle(pHSFileHandle);
	CloseHandle(pMSFileHandle);

	m_oRTShaderSet.insert(std::make_pair(a_sPath, pRTShaderGroup));
}

void D3DShaderManager::LoadRTShader(std::string const a_sPath, D3D_RT_SHADER_TYPE const a_eShaderType)
{
	D3DRTShader* pRTShader = nullptr;
	std::string sShaderPathSuffix;

	switch (a_eShaderType)
	{
		case RAYGEN:
			pRTShader = new D3DRayGenerationShader();
			sShaderPathSuffix = "_rg.cso";
			break;
		case HIT :
			pRTShader = new D3DHitShader();
			sShaderPathSuffix = "_ch.cso";
			break;
		case MISS:
			pRTShader = new D3DMissShader();
			sShaderPathSuffix = "_miss.cso";
			break;
	};
	assert(pRTShader != nullptr);

	std::string sPath = "./Shaders/" + a_sPath + sShaderPathSuffix;

	// Open shader bytecode
	HANDLE pFileHandle = CreateFileA(
		sPath.c_str(),
		GENERIC_READ,
		0,
		0,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	assert(pFileHandle != INVALID_HANDLE_VALUE);

	ULONG uiFileSize = GetFileSize(pFileHandle, nullptr);

	pRTShader->m_uiByteCodeSize = uiFileSize;
	pRTShader->m_pByteCode = new char[uiFileSize];

	BOOL bReadFileResult = ReadFile(pFileHandle, pRTShader->m_pByteCode, uiFileSize, NULL, NULL);
	assert(bReadFileResult != 0);

	CloseHandle(pFileHandle);

	switch (a_eShaderType)
	{
	case RAYGEN:
		m_oRayGenShaderSet.insert(std::make_pair(a_sPath, pRTShader));
		break;
	case HIT:
		m_oHitShaderSet.insert(std::make_pair(a_sPath, pRTShader));
		break;
	case MISS:
		m_oMissShaderSet.insert(std::make_pair(a_sPath, pRTShader));
		break;
	};
}

D3DShaderPair* D3DShaderManager::RequestShader(std::string const a_sPath)
{
	if (m_oShaderSet.find(a_sPath) == m_oShaderSet.end())
		LoadShader(a_sPath);

	return m_oShaderSet[a_sPath];
}

D3DRTShaderGroup* D3DShaderManager::RequestRTShader(std::string const a_sPath)
{
	if (m_oRTShaderSet.find(a_sPath) == m_oRTShaderSet.end())
		LoadRTShader(a_sPath);

	return m_oRTShaderSet[a_sPath];
}

D3DRTShader* D3DShaderManager::RequestRTShaderV2(std::string const a_sPath, D3D_RT_SHADER_TYPE const a_eShaderType)
{
	
	
	switch (a_eShaderType) // I'm sure there's a way to factorize this but i'm lazy, this will be enough for now
	{
		case(D3D_RT_SHADER_TYPE::RAYGEN):
		{
			if (m_oRayGenShaderSet.find(a_sPath) == m_oRayGenShaderSet.end())
				LoadRTShader(a_sPath, a_eShaderType);

			return m_oRayGenShaderSet[a_sPath];
		}
		break;
		case(D3D_RT_SHADER_TYPE::HIT):
		{
			if (m_oHitShaderSet.find(a_sPath) == m_oHitShaderSet.end())
				LoadRTShader(a_sPath, a_eShaderType);

			return m_oHitShaderSet[a_sPath];
		}
		break;
		case(D3D_RT_SHADER_TYPE::MISS):
		{
			if (m_oMissShaderSet.find(a_sPath) == m_oMissShaderSet.end())
				LoadRTShader(a_sPath, a_eShaderType);

			return m_oMissShaderSet[a_sPath];
		}
		break;
	};
}

UINT D3DShaderManager::GetRTShadersCount() const
{
	return m_oRayGenShaderSet.size() + m_oMissShaderSet.size() + m_oHitShaderSet.size();
}

D3DShaderManager g_D3DShaderManager;
