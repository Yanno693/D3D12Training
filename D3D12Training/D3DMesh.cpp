#include "D3DMesh.h"

#include "D3DDevice.h"
#include <map>
#include <fstream>
#include "json.hpp"

GameScreenResolution g_ScreenResolution;

std::vector<D3DMesh> D3DMesh::s_MeshList;

void D3DMesh::ParseObject(std::string a_sPath)
{
	using namespace tinyxml2;

	tinyxml2::XMLDocument oDoc;
	if (oDoc.LoadFile(a_sPath.c_str()) != XMLError::XML_SUCCESS)
	{
		std::stringstream ss;
		ss << "Error : Loading file " << a_sPath.c_str() << std::endl;
		OutputDebugStringA(ss.str().c_str());
		assert(0);
	}

	XMLElement* pXMLRoot = oDoc.RootElement();
	assert(pXMLRoot != nullptr);

	XMLElement* pXMLModel = pXMLRoot->FirstChildElement("model");
	assert(pXMLModel != nullptr);
	std::stringstream ssModelPath;
	ssModelPath << "./Models/" << pXMLModel->GetText();

	m_szModelPath = ssModelPath.str();

	XMLElement* pXMLShader = pXMLRoot->FirstChildElement("shader");
	assert(pXMLShader != nullptr);
	m_pShader = g_D3DShaderManager.RequestShader(pXMLShader->GetText());

	if (D3DDevice::isRayTracingEnabled())
	{
		XMLElement* pXMLRTShader = pXMLRoot->FirstChildElement("rtshader");
		assert(pXMLRTShader != nullptr);
		m_pHitShader = (D3DHitShader*)g_D3DShaderManager.RequestRTShaderV2(pXMLRTShader->GetText(), D3D_RT_SHADER_TYPE::HIT);
	}

	XMLElement* pXMLTransform = pXMLRoot->FirstChildElement("transform");
	assert(pXMLTransform != nullptr);

	XMLElement* apXMLTransformElements[3];
	apXMLTransformElements[0] = pXMLTransform->FirstChildElement("position");
	apXMLTransformElements[1] = pXMLTransform->FirstChildElement("rotation");
	apXMLTransformElements[2] = pXMLTransform->FirstChildElement("scale");

	float* pfTransformData[3];

	pfTransformData[0] = &m_oTransform.position.x;
	pfTransformData[1] = &m_oTransform.rotation.x;
	pfTransformData[2] = &m_oTransform.scale.x;

	for (int i = 0; i < 3; i++)
	{
		assert(apXMLTransformElements[i] != nullptr);

		std::stringstream ssX;
		std::stringstream ssY;
		std::stringstream ssZ;

		XMLElement* pXMLX = apXMLTransformElements[i]->FirstChildElement("x");
		XMLElement* pXMLY = apXMLTransformElements[i]->FirstChildElement("y");
		XMLElement* pXMLZ = apXMLTransformElements[i]->FirstChildElement("z");

		assert(pXMLX != nullptr);
		assert(pXMLY != nullptr);
		assert(pXMLZ != nullptr);

		ssX << pXMLX->GetText();
		ssY << pXMLY->GetText();
		ssZ << pXMLZ->GetText();

		*pfTransformData[i] = std::stof(ssX.str());
		pfTransformData[i]++;
		*pfTransformData[i] = std::stof(ssY.str());
		pfTransformData[i]++;
		*pfTransformData[i] = std::stof(ssZ.str());
		pfTransformData[i]++;
	}
}

void D3DMesh::ParseModel(std::string const a_sPath)
{
	using namespace tinyxml2;
	
	tinyxml2::XMLDocument oDoc;
	if (oDoc.LoadFile(a_sPath.c_str()) != XMLError::XML_SUCCESS)
	{
		std::stringstream ss;
		ss << "Error : Loading file " << a_sPath.c_str() << std::endl;
		OutputDebugStringA(ss.str().c_str());
		assert(0);
	}

	XMLElement* pXMLRoot = oDoc.RootElement();
	assert(pXMLRoot != nullptr);

	XMLElement* pColladaRoot = pXMLRoot->FirstChildElement("library_geometries");
	assert(pColladaRoot != nullptr);
	XMLElement* pGeometryRoot = pColladaRoot->FirstChildElement("geometry");
	assert(pGeometryRoot != nullptr);
	XMLElement* pXMLMeshRoot = pGeometryRoot->FirstChildElement("mesh");
	assert(pXMLMeshRoot != nullptr);

	std::string sGeometryId = pGeometryRoot->Attribute("id");
	assert(!sGeometryId.empty());

	struct XMLTriangleData
	{
		std::string semantic; // Sementic COLLADA side
		std::string source; // Eh ?
		int offset; // Offset COLLADA side

		std::vector<float> data;
		UINT dataStride;
		UINT dataCount;

	};

	// Get Triangle Data Indexes
	XMLElement* pXMLTriangle = pXMLMeshRoot->FirstChildElement("triangles");
	XMLElement* pXMLTriangleData = pXMLTriangle->FirstChildElement("p");

	std::string sTriangleCountString = pXMLTriangle->Attribute("count");
	assert(!sTriangleCountString.empty());
	m_uiTriangleCount = std::stoi(sTriangleCountString);
	assert(pXMLTriangle != nullptr);
	assert(pXMLTriangleData != nullptr);
	std::vector<XMLTriangleData> vTriangleSementicOffset;
	int uiLargestIndices = -1;

	// Get data for all sementics in std::vectors
	for (XMLElement* pInputIterator = pXMLTriangle->FirstChildElement("input"); pInputIterator != nullptr; pInputIterator = pInputIterator->NextSiblingElement("input"))
	{
		std::string sInputName = pInputIterator->Attribute("semantic");
		std::string sInputSource = pInputIterator->Attribute("source");
		assert(!sInputName.empty());
		assert(!sInputSource.empty());

		int iOffset;
		XMLError bIsOffsetValid = pInputIterator->QueryIntAttribute("offset", &iOffset);
		assert(bIsOffsetValid == XML_SUCCESS);

		if (sInputName == "VERTEX")
			sInputName = "POSITION";
		if (sInputSource.substr(1) == sGeometryId + "-vertices")
			sInputSource = "#" + sGeometryId + "-positions";

		vTriangleSementicOffset.push_back({ sInputName, sInputSource.substr(1), iOffset });
		uiLargestIndices = iOffset > uiLargestIndices ? iOffset : uiLargestIndices;
	}

	// Also a bit the same : have a std::vector for triangle data indexes
	std::vector<UINT> vTriangleData;
	{
		std::string sTriangleDataString = pXMLTriangleData->GetText();
		std::size_t uReadCharacters = 0;
		std::size_t uDataCurrentPosition = 0;

		while (uDataCurrentPosition < sTriangleDataString.size())
		{
			vTriangleData.push_back(std::stoi(sTriangleDataString.substr(uDataCurrentPosition), &uReadCharacters));
			uDataCurrentPosition += uReadCharacters;
		}
	}


	for (XMLElement* pSourceIterator = pXMLMeshRoot->FirstChildElement("source"); pSourceIterator != nullptr; pSourceIterator = pSourceIterator->NextSiblingElement("source"))
	{
		std::string sSourceIdIterator = pSourceIterator->Attribute("id");

		for (int i = 0; i < vTriangleSementicOffset.size(); i++)
		{
			if (sSourceIdIterator == vTriangleSementicOffset[i].source)
			{
				XMLElement* pXMLDataNode = pSourceIterator->FirstChildElement("float_array");
				assert(pXMLDataNode != nullptr);
				std::string sXMLDataNode = pXMLDataNode->GetText();
				std::size_t uReadCharacters = 0;
				std::size_t uDataCurrentPosition = 0;

				XMLElement* pXMLLayoutDataContainer = pSourceIterator->FirstChildElement("technique_common");
				assert(pXMLLayoutDataContainer != nullptr);
				XMLElement* pXMLLayoutData = pXMLLayoutDataContainer->FirstChildElement("accessor");
				assert(pXMLLayoutData != nullptr);

				std::string sDataStride = pXMLLayoutData->Attribute("stride");
				std::string sDataCount = pXMLLayoutData->Attribute("count");
				vTriangleSementicOffset[i].dataStride = std::stoi(sDataStride, nullptr);
				vTriangleSementicOffset[i].dataCount = std::stoi(sDataCount, nullptr);
				assert(vTriangleSementicOffset[i].dataStride > 0);
				assert(vTriangleSementicOffset[i].dataCount > 0);
				
				while (uDataCurrentPosition < sXMLDataNode.size())
				{
					vTriangleSementicOffset[i].data.push_back(std::stof(sXMLDataNode.substr(uDataCurrentPosition), &uReadCharacters));
					uDataCurrentPosition += uReadCharacters;
				}
			}
		}
	}

	std::vector<std::pair<size_t, std::string>> oDataOffsetSementicMap;

	// Get the size of one vertex, this will help to allocate some memory to copy to the vertex buffer.
	size_t uiVertexSize = 0;
	for (int i = 0; i < m_pShader->m_vInputElements.size(); i++)
	{
		std::string sLayoutSementic = m_pShader->m_vInputElements[i].SemanticName;
		
		for (int j = 0; j < vTriangleSementicOffset.size(); j++)
		{
			std::string sLoadedMeshSementic = vTriangleSementicOffset[j].semantic;

			if (sLayoutSementic == sLayoutSementic)
			{
				oDataOffsetSementicMap.push_back(std::make_pair(uiVertexSize, sLayoutSementic)); // This way we'll keep the order of each data, and their location in the vertex
				uiVertexSize += GetSizeFromDXGIFormat(m_pShader->m_vInputElements[i].Format) / 8; // This function return the size in bit, so we divide by 8 for bytes
				j = (int)vTriangleSementicOffset.size(); // To get out of the loop faster
			}
		}
	}
	
	// Allocate data to copy to buffer
	const size_t uiVertexBufferSize = uiVertexSize * m_uiTriangleCount * 3;
	char* pVertexBufferData = (char*)malloc(uiVertexBufferSize);
	char* pVertexBufferDataIterator = pVertexBufferData;
	assert(pVertexBufferData != nullptr);


	for (UINT i = 0; i < m_uiTriangleCount * 3; i++) // < For each vertex
	{
		for (std::pair<size_t, std::string>& roDataOffsetSementic : oDataOffsetSementicMap)
		{
			std::string sSementicToFetch = roDataOffsetSementic.second;
			UINT uiSementicOffset = 0xFFFFFFFF;
			UINT uiSementicIndex = 0xFFFFFFFF;
			size_t uiSementicStride = 0xFFFFFFFF;
			for (int j = 0; j < vTriangleSementicOffset.size(); j++)
			{
				if (sSementicToFetch == vTriangleSementicOffset[j].semantic)
				{
					uiSementicOffset = vTriangleSementicOffset[j].offset;
					uiSementicStride = vTriangleSementicOffset[j].dataStride * sizeof(float);
					uiSementicIndex = j;
					j = (int)vTriangleSementicOffset.size(); // Quick exit
				}
			}
			assert(uiSementicOffset != 0xFFFFFFFF);
			assert(uiSementicIndex != 0xFFFFFFFF);
			assert(uiSementicStride != 0xFFFFFFFF);
			UINT uiDataIndexToFetchInTriangleData = vTriangleData[i * (uiLargestIndices + 1) + uiSementicOffset];

			// Begining of Data + Index * size of the data to copy
			char* pDataToCopy = ((char*)vTriangleSementicOffset[uiSementicIndex].data.data()) + uiSementicStride * uiDataIndexToFetchInTriangleData;

			memcpy(pVertexBufferDataIterator, pDataToCopy, uiSementicStride);
			pVertexBufferDataIterator += uiSementicStride;
			assert(pVertexBufferDataIterator <= pVertexBufferData + uiVertexBufferSize);
		}
	}

	// Now copy to vertex buffer
	g_D3DBufferManager.InitializeVertexBuffer(&m_oVertexBuffer, (UINT)uiVertexBufferSize, (UINT)uiVertexSize);
	m_oVertexBuffer.WriteData(pVertexBufferData, (UINT)uiVertexBufferSize);

	free(pVertexBufferData);	
}

size_t GetGLTFTypeSize(UINT a_uiType)
{
	switch (a_uiType)
	{
	case 5120:
	case 5121:
		return sizeof(BYTE);
		break;
	case 5122:
	case 5123:
		return sizeof(USHORT);
		break;
	default:
		return sizeof(UINT);
		break;
	}
}

void D3DMesh::ParseModelGLTF(std::string const a_sPath, std::string const a_sPathBin)
{
	using json = nlohmann::json;
	
	std::ifstream oGLTFStream(a_sPath);
	std::ifstream oGLTFBinStream(a_sPathBin, std::ios::binary | :: std::ios::ate);
	assert(oGLTFStream);
	assert(oGLTFBinStream);

	char* oGLTFBinData = (char*)malloc(oGLTFBinStream.tellg());
	assert(oGLTFBinData != nullptr);
	size_t oGLTFBinDataLength = oGLTFBinStream.tellg();
	oGLTFBinStream.seekg(0);
	oGLTFBinStream.read(oGLTFBinData, oGLTFBinDataLength);
	oGLTFBinStream.close();

	json oGLTFFileJson = json::parse(oGLTFStream);
	assert(!oGLTFFileJson.empty());

	UINT oGLTFPositionAccessorIndex = oGLTFFileJson["meshes"][0]["primitives"][0]["attributes"]["POSITION"].get<UINT>();
	UINT oGLTFPositionBufferViewID = oGLTFFileJson["accessors"][oGLTFPositionAccessorIndex]["bufferView"].get<UINT>();
	UINT oGLTFPositionCount = oGLTFFileJson["accessors"][oGLTFPositionAccessorIndex]["count"].get<UINT>();
	UINT oGLTFPositionDataInBuffer = oGLTFFileJson["bufferViews"][oGLTFPositionBufferViewID]["byteOffset"].get<UINT>();
	UINT oGLTFPositionLengthInBuffer = oGLTFFileJson["bufferViews"][oGLTFPositionBufferViewID]["byteLength"].get<UINT>();

	UINT oGLTFUVAccessorIndex = oGLTFFileJson["meshes"][0]["primitives"][0]["attributes"]["TEXCOORD_0"].get<UINT>();
	UINT oGLTFUVBufferViewID = oGLTFFileJson["accessors"][oGLTFUVAccessorIndex]["bufferView"].get<UINT>();
	UINT oGLTFUVCount = oGLTFFileJson["accessors"][oGLTFUVAccessorIndex]["count"].get<UINT>();
	UINT oGLTFUVDataInBuffer = oGLTFFileJson["bufferViews"][oGLTFUVBufferViewID]["byteOffset"].get<UINT>();
	UINT oGLTFUVLengthInBuffer = oGLTFFileJson["bufferViews"][oGLTFUVBufferViewID]["byteLength"].get<UINT>();

	UINT oGLTFNormalAccessorIndex = oGLTFFileJson["meshes"][0]["primitives"][0]["attributes"]["NORMAL"].get<UINT>();
	UINT oGLTFNormalBufferViewID = oGLTFFileJson["accessors"][oGLTFNormalAccessorIndex]["bufferView"].get<UINT>();
	UINT oGLTFNormalCount = oGLTFFileJson["accessors"][oGLTFNormalAccessorIndex]["count"].get<UINT>();
	UINT oGLTFNormalDataInBuffer = oGLTFFileJson["bufferViews"][oGLTFNormalBufferViewID]["byteOffset"].get<UINT>();
	UINT oGLTFNormalLengthInBuffer = oGLTFFileJson["bufferViews"][oGLTFNormalBufferViewID]["byteLength"].get<UINT>();

	assert(oGLTFPositionCount == oGLTFUVCount && oGLTFPositionCount == oGLTFNormalCount);

	UINT oGLTFIndicesAccessorIndex = oGLTFFileJson["meshes"][0]["primitives"][0]["indices"].get<UINT>();
	UINT oGLTFIndicesBufferViewID = oGLTFFileJson["accessors"][oGLTFIndicesAccessorIndex]["bufferView"].get<UINT>();
	UINT oGLTFIndicesCount = oGLTFFileJson["accessors"][oGLTFIndicesAccessorIndex]["count"].get<UINT>();
	UINT oGLTFIndicesType = oGLTFFileJson["accessors"][oGLTFIndicesAccessorIndex]["componentType"].get<UINT>();
	UINT oGLTFIndicesDataInBuffer = oGLTFFileJson["bufferViews"][oGLTFIndicesBufferViewID]["byteOffset"].get<UINT>();
	UINT oGLTFIndicesLengthInBuffer = oGLTFFileJson["bufferViews"][oGLTFIndicesBufferViewID]["byteLength"].get<UINT>();

	m_oMeshPositionData.ptr = (char*)malloc(oGLTFPositionLengthInBuffer);
	assert(m_oMeshPositionData.ptr != nullptr);
	memcpy(m_oMeshPositionData.ptr, oGLTFBinData + oGLTFPositionDataInBuffer, oGLTFPositionLengthInBuffer);
	m_oMeshPositionData.size = oGLTFPositionLengthInBuffer;
	m_oMeshPositionData.stride = oGLTFPositionLengthInBuffer / oGLTFPositionCount;
	m_oMeshPositionData.count = oGLTFPositionCount;

	m_oMeshUVData.ptr = (char*)malloc(oGLTFUVLengthInBuffer);
	assert(m_oMeshUVData.ptr != nullptr);
	memcpy(m_oMeshUVData.ptr, oGLTFBinData + oGLTFUVDataInBuffer, oGLTFUVLengthInBuffer);
	m_oMeshUVData.size = oGLTFUVLengthInBuffer;
	m_oMeshUVData.stride = oGLTFUVLengthInBuffer / oGLTFUVCount;
	m_oMeshUVData.count = oGLTFUVCount;

	m_oMeshNormalData.ptr = (char*)malloc(oGLTFNormalLengthInBuffer);
	assert(m_oMeshNormalData.ptr != nullptr);
	memcpy(m_oMeshNormalData.ptr, oGLTFBinData + oGLTFNormalDataInBuffer, oGLTFNormalLengthInBuffer);
	m_oMeshNormalData.size = oGLTFNormalLengthInBuffer;
	m_oMeshNormalData.stride = oGLTFNormalLengthInBuffer / oGLTFNormalCount;
	m_oMeshNormalData.count = oGLTFNormalCount;

	m_oMeshIndicesData.ptr = (char*)malloc(oGLTFIndicesLengthInBuffer);
	assert(m_oMeshIndicesData.ptr != nullptr);
	memcpy(m_oMeshIndicesData.ptr, oGLTFBinData + oGLTFIndicesDataInBuffer, oGLTFIndicesLengthInBuffer);
	m_oMeshIndicesData.size = oGLTFIndicesLengthInBuffer;
	m_oMeshIndicesData.stride = (UINT)GetGLTFTypeSize(oGLTFIndicesType);
	m_oMeshIndicesData.count = oGLTFIndicesCount;

	m_uiIndicesCount = oGLTFIndicesCount;
	m_uiTriangleCount = oGLTFPositionCount / 3;

	free(oGLTFBinData);
	oGLTFStream.close();
}

void D3DMesh::CreateGPUBuffers()
{
	UINT uiVertexBufferSize = m_oMeshPositionData.size + m_oMeshNormalData.size;
	UINT uiVertexStride = m_oMeshPositionData.stride + m_oMeshNormalData.stride;
	char* oVertexData = (char*)malloc(uiVertexBufferSize);
	assert(oVertexData != nullptr);

	char* oVertexDataIt = oVertexData;

	for (UINT i = 0; i < m_oMeshPositionData.count; i++)
	{
		memcpy(oVertexDataIt, m_oMeshPositionData.ptr + i * m_oMeshPositionData.stride, m_oMeshPositionData.stride);
		memcpy(oVertexDataIt + m_oMeshPositionData.stride, m_oMeshNormalData.ptr + i * m_oMeshNormalData.stride, m_oMeshNormalData.stride);

		oVertexDataIt += m_oMeshPositionData.stride + m_oMeshNormalData.stride;
	}

	g_D3DBufferManager.InitializeVertexBuffer(&m_oVertexBuffer, uiVertexBufferSize, uiVertexStride);
	m_oVertexBuffer.WriteData(oVertexData, uiVertexBufferSize);

	g_D3DBufferManager.InitializeIndexBuffer(&m_oIndexBuffer, m_oMeshIndicesData.size, m_oMeshIndicesData.stride == sizeof(USHORT));
	m_oIndexBuffer.WriteData(m_oMeshIndicesData.ptr, m_oMeshIndicesData.size);

	g_D3DBufferManager.InitializeConstantBuffer(&m_oInstanceBuffer, sizeof(MeshConstant));

	free(oVertexData);
}

void D3DMesh::CreateRTGPUBuffers(ID3D12Device5* a_pDevice)
{
	if (!D3DDevice::isRayTracingEnabled())
		return;

	UINT uiVertexSize = m_oMeshPositionData.size + m_oMeshNormalData.size;
	UINT uiVertexStride = m_oMeshPositionData.stride + m_oMeshNormalData.stride;
	char* oVertexData = (char*)malloc(uiVertexSize);
	assert(oVertexData != nullptr);

	char* oVertexDataIt = oVertexData;

	for (UINT i = 0; i < m_oMeshPositionData.count; i++)
	{
		memcpy(oVertexDataIt, m_oMeshPositionData.ptr + i * m_oMeshPositionData.stride, m_oMeshPositionData.stride);
		memcpy(oVertexDataIt + m_oMeshPositionData.stride, m_oMeshNormalData.ptr + i * m_oMeshNormalData.stride, m_oMeshNormalData.stride);

		oVertexDataIt += m_oMeshPositionData.stride + m_oMeshNormalData.stride;
	}
	
	g_D3DBufferManager.InitializeGenericBuffer(&m_oRTVertexBuffer.m_oBuffer, uiVertexSize);
	m_oRTVertexBuffer.m_oData.SizeInBytes = uiVertexSize;
	m_oRTVertexBuffer.m_oData.StrideInBytes = uiVertexStride;
	m_oRTVertexBuffer.WriteData(oVertexData, uiVertexSize);

	g_D3DBufferManager.InitializeGenericBuffer(&m_oRTIndexBuffer.m_oBuffer, m_oMeshIndicesData.size);
	m_oRTIndexBuffer.m_oData.SizeInBytes = m_oMeshIndicesData.size;
	m_oRTIndexBuffer.m_oData.Format = (m_oMeshIndicesData.stride == sizeof(USHORT) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
	m_oRTIndexBuffer.WriteData(m_oMeshIndicesData.ptr, m_oMeshIndicesData.size);

	// Create BVH
	m_oBVHGeometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	m_oBVHGeometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	m_oBVHGeometry.Triangles.Transform3x4 = 0;
	m_oBVHGeometry.Triangles.IndexFormat = m_oRTIndexBuffer.m_oData.Format;
	m_oBVHGeometry.Triangles.IndexCount = m_oMeshIndicesData.count;
	m_oBVHGeometry.Triangles.IndexBuffer = m_oRTIndexBuffer.m_oBuffer.m_pResource.Get()->GetGPUVirtualAddress();
	m_oBVHGeometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	m_oBVHGeometry.Triangles.VertexCount = m_oMeshPositionData.count;
	m_oBVHGeometry.Triangles.VertexBuffer.StartAddress = m_oRTVertexBuffer.m_oBuffer.m_pResource.Get()->GetGPUVirtualAddress();
	m_oBVHGeometry.Triangles.VertexBuffer.StrideInBytes = m_oRTVertexBuffer.m_oData.StrideInBytes;

	m_oBVHInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	m_oBVHInput.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	m_oBVHInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	m_oBVHInput.NumDescs = 1;
	m_oBVHInput.pGeometryDescs = &m_oBVHGeometry;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO oBVHPreBuildInfo;
	a_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&m_oBVHInput, &oBVHPreBuildInfo);

	g_D3DBufferManager.InitializeGenericBuffer(&m_oBVH, (UINT)oBVHPreBuildInfo.ResultDataMaxSizeInBytes, true, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
	g_D3DBufferManager.InitializeGenericBuffer(&m_oBVHScratch, (UINT)oBVHPreBuildInfo.ScratchDataSizeInBytes, true);

	free(oVertexData);
}


void D3DMesh::InitializeDebug(ID3D12Device5* a_pDevice, bool a_bUsesRayTracing)
{
	if (a_bUsesRayTracing)
	{
		// 1. Load Shader
		m_pHitShader = (D3DHitShader*)g_D3DShaderManager.RequestRTShaderV2("basicsolidrt", D3D_RT_SHADER_TYPE::HIT);

		// 2. Fill With something
		_3DVertex vVertex[3] = {
			{0, 0, 5.0f},
			{0, 1, 5.0f},
			{1, 0, 5.0f}
		};

		UINT32 vIndex[3] = { 0, 1, 2 };

		m_uiTriangleCount = 1;

		g_D3DBufferManager.InitializeGenericBuffer(&m_oRTVertexBuffer.m_oBuffer, sizeof(vVertex));
		m_oRTVertexBuffer.m_oData.SizeInBytes = sizeof(vVertex);
		m_oRTVertexBuffer.m_oData.StrideInBytes = sizeof(_3DVertex);
		m_oRTVertexBuffer.WriteData(vVertex, sizeof(vVertex));
		g_D3DBufferManager.InitializeGenericBuffer(&m_oRTIndexBuffer.m_oBuffer, sizeof(vIndex));
		m_oRTIndexBuffer.m_oData.SizeInBytes = sizeof(vIndex);
		m_oRTIndexBuffer.m_oData.Format = DXGI_FORMAT_R32_UINT;
		m_oRTIndexBuffer.WriteData(vIndex, sizeof(vIndex));

		// Create BVH
		m_oBVHGeometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		m_oBVHGeometry.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		m_oBVHGeometry.Triangles.Transform3x4 = 0;
		m_oBVHGeometry.Triangles.IndexFormat = m_oRTIndexBuffer.m_oData.Format;
		m_oBVHGeometry.Triangles.IndexCount = 3 * m_uiTriangleCount;
		m_oBVHGeometry.Triangles.IndexBuffer = m_oRTIndexBuffer.m_oBuffer.m_pResource.Get()->GetGPUVirtualAddress();
		m_oBVHGeometry.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		m_oBVHGeometry.Triangles.VertexCount = _countof(vVertex);
		m_oBVHGeometry.Triangles.VertexBuffer.StartAddress = m_oRTVertexBuffer.m_oBuffer.m_pResource.Get()->GetGPUVirtualAddress();
		m_oBVHGeometry.Triangles.VertexBuffer.StrideInBytes = m_oRTVertexBuffer.m_oData.StrideInBytes;

		m_oBVHInput.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		m_oBVHInput.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		m_oBVHInput.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		m_oBVHInput.NumDescs = 1;
		m_oBVHInput.pGeometryDescs = &m_oBVHGeometry;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO oBVHPreBuildInfo;
		a_pDevice->GetRaytracingAccelerationStructurePrebuildInfo(&m_oBVHInput,&oBVHPreBuildInfo);

		g_D3DBufferManager.InitializeGenericBuffer(&m_oBVH, (UINT)oBVHPreBuildInfo.ResultDataMaxSizeInBytes, true, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
		m_oBVH.SetDebugName(L"BVH Buffer");
		g_D3DBufferManager.InitializeGenericBuffer(&m_oBVHScratch, (UINT)oBVHPreBuildInfo.ScratchDataSizeInBytes, true);
		m_oBVHScratch.SetDebugName(L"BVH Scratch Buffer");

	}
	else
	{
		// 1. Load shader
		m_pShader = g_D3DShaderManager.RequestShader("basicsolid");

		// 2. Fill With something
		_3DVertex vVertex[3] = {
			{0, 0, 5.0f},
			{0, 1, 5.0f},
			{1, 0, 5.0f}
		};

		UINT32 vIndex[3] = { 0, 1, 2 };

		m_uiTriangleCount = 1;

		g_D3DBufferManager.InitializeVertexBuffer(&m_oVertexBuffer, sizeof(vVertex), sizeof(_3DVertex));
		m_oVertexBuffer.WriteData((void*)vVertex, sizeof(vVertex));
		g_D3DBufferManager.InitializeIndexBuffer(&m_oIndexBuffer, sizeof(vIndex));
		m_oIndexBuffer.WriteData((void*)vIndex, sizeof(vIndex));

		// Set debug name
		std::wstring ws(L"Debug Geometry");
		std::wstringstream ss;
		ss << ws << " Vertex Buffer";
		m_oVertexBuffer.SetDebugName(ss.str().c_str());

		// Create PSO

		D3D12_INPUT_LAYOUT_DESC oInputLayoutDesc = {};
		oInputLayoutDesc.NumElements = (UINT)m_pShader->m_vInputElements.size();
		oInputLayoutDesc.pInputElementDescs = m_pShader->m_vInputElements.data();

		D3D12_SHADER_BYTECODE oDxVs;
		oDxVs.BytecodeLength = m_pShader->m_pVS->m_uiByteCodeSize;
		oDxVs.pShaderBytecode = m_pShader->m_pVS->m_pByteCode;

		D3D12_SHADER_BYTECODE oDxPs;
		oDxPs.BytecodeLength = m_pShader->m_pPS->m_uiByteCodeSize;
		oDxPs.pShaderBytecode = m_pShader->m_pPS->m_pByteCode;

		D3D12_ROOT_PARAMETER oVBSceneRootParameter = {};
		oVBSceneRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		oVBSceneRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER oVBPerObjectRootParameter = {};
		oVBPerObjectRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		oVBPerObjectRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_PARAMETER pVBRootParameters[] = { oVBSceneRootParameter , oVBPerObjectRootParameter };

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
		rsDesc.NumParameters = 1;
		rsDesc.NumStaticSamplers = 0;
		rsDesc.pStaticSamplers = NULL;
		rsDesc.pParameters = pVBRootParameters; // TODO : Use a list, could be better
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		Microsoft::WRL::ComPtr<ID3DBlob> rsBlob = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

		D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
		if (errorBlob != nullptr)
		{
			void* d = errorBlob->GetBufferPointer();
			int a = 1;
			assert(0);
		}

		if (!SUCCEEDED(a_pDevice->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
		{
			OutputDebugStringA("Error : Create Empty Root signature\n");
			assert(0);
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC oPsoDesc = {};
		oPsoDesc.InputLayout = oInputLayoutDesc;
		oPsoDesc.NumRenderTargets = 1;
		oPsoDesc.SampleDesc.Count = 1;
		oPsoDesc.SampleMask = 0xFFFFFFFF;
		oPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		oPsoDesc.BlendState.RenderTarget[0].BlendEnable = true;
		oPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		oPsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		oPsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		oPsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_COLOR;
		oPsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		oPsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
		oPsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

		oPsoDesc.VS = oDxVs;
		oPsoDesc.PS = oDxPs;
		oPsoDesc.DepthStencilState.DepthEnable = false;
		oPsoDesc.DepthStencilState.StencilEnable = false;
		oPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		oPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		oPsoDesc.pRootSignature = m_pRootSignature.Get();
		oPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		if (!SUCCEEDED(a_pDevice->CreateGraphicsPipelineState(&oPsoDesc, IID_PPV_ARGS(&m_pPso))))
		{
			std::stringstream sErrorSS;
			sErrorSS << "Error : Pipeline state object compilation for debug object" << "\n";
			OutputDebugStringA(sErrorSS.str().c_str());
			assert(0);
		}
	}
}

void D3DMesh::Initialize(std::string a_sPath, ID3D12Device5* a_pDevice, bool a_bUsesRayTracing)
{
	std::stringstream sObjectPath;
	std::stringstream sModelPath;
	std::stringstream sModelPathGLTF;
	std::stringstream sModelPathGLTFBin;
	sObjectPath << "./Objects/" << a_sPath << ".xml";
	sModelPath << "./Models/" << a_sPath << ".dae";
	sModelPathGLTF << "./Models/" << a_sPath << ".gltf";
	sModelPathGLTFBin << "./Models/" << a_sPath << ".bin";

	assert(m_pShader == nullptr);
	
	ParseObject(sObjectPath.str());
	ParseModelGLTF(m_szModelPath + ".gltf", m_szModelPath + ".bin");

	CreateGPUBuffers();

	// Set debug name
	std::wstring ws(a_sPath.begin(), a_sPath.end());
	std::wstringstream ssVertexBuffer;
	ssVertexBuffer << ws << " Vertex Buffer";
	std::wstringstream ssRTVertexBuffer;
	m_oVertexBuffer.SetDebugName(ssVertexBuffer.str().c_str());

	if (D3DDevice::isRayTracingEnabled())
	{
		CreateRTGPUBuffers(a_pDevice);

		ssRTVertexBuffer << ws << " RT Vertex Buffer";
		std::wstringstream ssRTIndexBuffer;
		ssRTIndexBuffer << ws << " RT Index Buffer";
		std::wstringstream ssRTBVH;
		ssRTBVH << ws << " BVH";
		std::wstringstream ssRTBVHScratch;
		ssRTBVHScratch << ws << " BVH Scratch";

		m_oRTVertexBuffer.SetDebugName(ssRTVertexBuffer.str().c_str());
		m_oRTIndexBuffer.SetDebugName(ssRTIndexBuffer.str().c_str());
		m_oBVH.SetDebugName(ssRTBVH.str().c_str());
		m_oBVHScratch.SetDebugName(ssRTBVHScratch.str().c_str());
	}

	// Create PSO
	D3D12_INPUT_LAYOUT_DESC oInputLayoutDesc = {};
	oInputLayoutDesc.NumElements = (UINT)m_pShader->m_vInputElements.size();
	oInputLayoutDesc.pInputElementDescs = m_pShader->m_vInputElements.data();

	D3D12_SHADER_BYTECODE oDxVs;
	oDxVs.BytecodeLength = m_pShader->m_pVS->m_uiByteCodeSize;
	oDxVs.pShaderBytecode = m_pShader->m_pVS->m_pByteCode;

	D3D12_SHADER_BYTECODE oDxPs;
	oDxPs.BytecodeLength = m_pShader->m_pPS->m_uiByteCodeSize;
	oDxPs.pShaderBytecode = m_pShader->m_pPS->m_pByteCode;

	D3D12_ROOT_PARAMETER oVBSceneRootParameter = {};
	oVBSceneRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	oVBSceneRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	oVBSceneRootParameter.Descriptor.ShaderRegister = 0;
	oVBSceneRootParameter.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER oVBPerObjectRootParameter = {};
	oVBPerObjectRootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	oVBPerObjectRootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	oVBPerObjectRootParameter.Descriptor.ShaderRegister = 1;
	oVBPerObjectRootParameter.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_PARAMETER pVBRootParameters[] = { oVBSceneRootParameter , oVBPerObjectRootParameter };

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = _countof(pVBRootParameters);
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = NULL;
	rsDesc.pParameters = pVBRootParameters;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> rsBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	D3D12SerializeRootSignature(&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rsBlob, &errorBlob);
	if (errorBlob != nullptr)
	{
		void* d = errorBlob->GetBufferPointer();
		int a = 1;
		assert(0);
	}

	if (!SUCCEEDED(a_pDevice->CreateRootSignature(0, rsBlob->GetBufferPointer(), rsBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
	{
		OutputDebugStringA("Error : Create Empty Root signature\n");
		assert(0);
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC oPsoDesc = {};
	oPsoDesc.InputLayout = oInputLayoutDesc;
	oPsoDesc.NumRenderTargets = 1;
	oPsoDesc.SampleDesc.Count = 1;
	oPsoDesc.SampleMask = 0xFFFFFFFF;
	oPsoDesc.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	oPsoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	oPsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	oPsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	oPsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	oPsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_COLOR;
	oPsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	oPsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_DEST_COLOR;
	oPsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	oPsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	oPsoDesc.DepthStencilState.DepthEnable = true;
	oPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

	oPsoDesc.VS = oDxVs;
	oPsoDesc.PS = oDxPs;
	//oPsoDesc.DepthStencilState.DepthEnable = false;
	oPsoDesc.DepthStencilState.StencilEnable = false;
	oPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	oPsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	oPsoDesc.pRootSignature = m_pRootSignature.Get();
	oPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	
	if (!SUCCEEDED(a_pDevice->CreateGraphicsPipelineState(&oPsoDesc, IID_PPV_ARGS(&m_pPso))))
	{
		std::stringstream sErrorSS;
		sErrorSS << "Error : Pipeline state object compilation for object " << a_sPath << "\n";
		OutputDebugStringA(sErrorSS.str().c_str());
		assert(0);
	}
}

void D3DMesh::Draw(ID3D12GraphicsCommandList* a_pCommandList)
{
	// TODO : To move in a sort of Update() fuction
	DirectX::XMMATRIX oTranslationMatrix = DirectX::XMMatrixTranslation(m_oTransform.position.x, m_oTransform.position.y, m_oTransform.position.z);
	DirectX::XMMATRIX oScaleMatrix = DirectX::XMMatrixScaling(m_oTransform.scale.x, m_oTransform.scale.y, m_oTransform.scale.z);
	DirectX::XMMATRIX oTransform = oScaleMatrix * oTranslationMatrix;

	MeshConstant oConstant = {};
	oConstant.mModelMatrix = oTransform;

	m_oInstanceBuffer.WriteData(&oConstant, sizeof(oConstant));
	
	D3D12_VERTEX_BUFFER_VIEW vViews[1] = { m_oVertexBuffer.m_oView };
	
	a_pCommandList->IASetVertexBuffers(0, 1, vViews);
	a_pCommandList->IASetIndexBuffer(&m_oIndexBuffer.m_oView);
	a_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

	a_pCommandList->SetGraphicsRootConstantBufferView(0, g_SceneConstantBuffer.m_pResource->GetGPUVirtualAddress());
	a_pCommandList->SetGraphicsRootConstantBufferView(1, m_oInstanceBuffer.m_pResource->GetGPUVirtualAddress()); // Todo : implement instance buffer
	//a_pCommandList->SetGraphicsRootDescriptorTable(0, g_SceneConstantBuffer.m_eGPUHandle);


	D3D12_RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = g_ScreenResolution.width;
	rect.bottom = g_ScreenResolution.height;

	D3D12_VIEWPORT viewport;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (FLOAT)g_ScreenResolution.width;
	viewport.Height = (FLOAT)g_ScreenResolution.height;
	a_pCommandList->RSSetScissorRects(1, &rect);
	a_pCommandList->RSSetViewports(1, &viewport);
	a_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	a_pCommandList->SetPipelineState(m_pPso.Get());
	//a_pCommandList->DrawInstanced(3 * m_uiTriangleCount, 1, 0, 0);

	if (m_uiIndicesCount != 0)
	{
		a_pCommandList->DrawIndexedInstanced(m_uiIndicesCount, 1, 0, 0, 0);
	}
	else
	{
		a_pCommandList->DrawInstanced(3 * m_uiTriangleCount, 1, 0, 0);
	}
}

void D3DMesh::DrawRT(ID3D12GraphicsCommandList4* a_pCommandList)
{
	/*
	a_pCommandList->SetPipelineState1(m_pRayTracingPso.Get());

	D3D12_DISPATCH_RAYS_DESC oRayDispatch = {};
	oRayDispatch.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.RayGenerationShaderRecord.StartAddress = m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress();
	oRayDispatch.MissShaderTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.MissShaderTable.StartAddress = m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 1 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.HitGroupTable.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	oRayDispatch.HitGroupTable.StartAddress = m_oShaderIDBuffer.m_pResource.Get()->GetGPUVirtualAddress() + 2 * D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	oRayDispatch.Width = 1280;
	oRayDispatch.Height = 720;
	oRayDispatch.Depth = 1;

	a_pCommandList->DispatchRays(&oRayDispatch);
	*/
}

void D3DMesh::SetPosition(const GamePosition& a_rPosition)
{
	m_oTransform.position = a_rPosition;
}

GamePosition D3DMesh::GetPosition()
{
	return m_oTransform.position;
}

void D3DMesh::SetScale(const GameScale& a_rScale)
{
	m_oTransform.scale = a_rScale;
}

GameScale D3DMesh::GetScale()
{
	return m_oTransform.scale;
}
