#pragma once
#include "D3DIncludes.h"

#include "D3DVertexBuffer.h"
#include "D3DIndexBuffer.h"
#include "D3DConstantBuffer.h"

#include "D3DShaderManager.h"
#include "D3DBufferManager.h"

#include "tinyxml2.h"

struct ExtraPtr
{
	char* ptr = nullptr;
	UINT size = 0;
	UINT stride = 0;
	UINT count = 0;
};

class D3DMesh
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;


	GameTransform m_oTransform;

	void ParseObject(std::string a_sPath); // Load the object
	void ParseModel(std::string a_sPath); // Load the geometry of the object (ParseObject has to be called first)
	void ParseModelGLTF(std::string a_sPath, std::string const a_sPathBin); // Load the geometry of the object from a GLTF file (ParseObject has to be called first)

	void CreateGPUBuffers();
	void CreateRTGPUBuffers(ID3D12Device5* a_pDevice);

	ExtraPtr m_oMeshPositionData = {};
	ExtraPtr m_oMeshUVData = {};
	ExtraPtr m_oMeshIndicesData = {};

	/*
	UINT m_uiVertexElemCount = 0;
	UINT m_uiVertexPosSize = 0;
	UINT m_uiVertexUVSize = 0;
	char* m_uiMeshVertexPositionData = 0;
	*/

	UINT m_uiTriangleCount = 0; // Number of triangle in the mesh
	UINT m_uiIndicesCount = 0; // Number of indices for the mesh vertices

public:
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_pRayTracingPso; // TODO : PRIVATE !
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRayTracingRootSignature; // TODO : PRIVATE !
	// Raster Stuff
	D3DVertexBuffer m_oVertexBuffer; // Vertex Buffer of the Mesh
	D3DIndexBuffer m_oIndexBuffer; // Index buffer ot the mesh
	D3DShaderPair* m_pShader = nullptr;

	// Ray Tracing Stuff
	D3DRayTracingVertexBuffer m_oRTVertexBuffer; // Vertex Buffer the Mesh for ray tracing
	D3DRayTracingIndexBuffer m_oRTIndexBuffer; // Index buffer ot the mesh
	D3DGenericBuffer m_oBVH; // "Bottom Level Acceleration Stucture", basically a sort of BVH
	D3DGenericBuffer m_oBVHScratch; // "Bottom Level Acceleration Stucture" but for updating ?
	D3DGenericBuffer m_oShaderIDBuffer; // Buffer holding Shader IDs for ray tracing
	UINT m_uiShaderIDCount = 0;
	D3D12_RAYTRACING_GEOMETRY_DESC m_oBVHGeometry = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS m_oBVHInput = {};
	D3DRTShaderGroup* m_pRTShader = nullptr;
	ID3D12Resource* shaderIDs;

	D3DConstantBuffer m_oInstanceBuffer; // Constant Buffer for the mesh, contains the transform of the object (on the Model matrix)


	void Initialize(std::string const a_sPath, ID3D12Device5* a_pDevice, bool a_bUsesRayTracing = false);
	void InitializeDebug(ID3D12Device5* a_pDevice, bool a_bUsesRayTracing = false);
	void Draw(ID3D12GraphicsCommandList* a_pCommandList);
	void DrawRT(ID3D12GraphicsCommandList4* a_pCommandList);

	void SetPosition(const GamePosition& a_rPosition);
	GamePosition GetPosition();

	bool m_bIsIndexed = false;
};

extern std::vector<D3DMesh> g_MeshList;

