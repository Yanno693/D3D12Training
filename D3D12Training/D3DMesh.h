#pragma once
#include "D3DIncludes.h"

#include "D3DVertexBuffer.h"
#include "D3DIndexBuffer.h"
#include "D3DConstantBuffer.h"

#include "D3DShaderManager.h"
#include "D3DBufferManager.h"

#include "ShaderShared.h"

#include "tinyxml2.h"

struct ExtraPtr
{
	char* ptr = nullptr;
	UINT size = 0;
	UINT stride = 0;
	UINT count = 0;
};

struct MeshConstant
{
	DirectX::XMMATRIX mModelMatrix;
};

class D3DMesh
{
private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPso;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;

	GameTransform m_oTransform;
	std::string m_szModelPath; // Path of the model to load without extension

	void ParseObject(std::string a_sPath); // Load the object
	void ParseModelGLTF(std::string a_sPath, std::string const a_sPathBin); // Load the geometry of the object from a GLTF file (ParseObject has to be called first)

	void CreateGPUBuffers();
	void CreateRTGPUBuffers(ID3D12Device5* a_pDevice);
	void CreatePSO(ID3D12Device* a_pDevice);

	ExtraPtr m_oMeshPositionData = {};
	ExtraPtr m_oMeshUVData = {};
	ExtraPtr m_oMeshNormalData = {};
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
	// Raster Stuff
	D3DVertexBuffer m_oVertexBuffer; // Vertex Buffer of the Mesh
	D3DIndexBuffer m_oIndexBuffer; // Index buffer ot the mesh
	D3DConstantBuffer m_oInstanceBuffer; // Instance buffer of the mesh
	D3DShaderPair* m_pShader = nullptr;

	// Ray Tracing Stuff
	D3DGenericBuffer m_oBVH; // "Bottom Level Acceleration Stucture", basically a sort of BVH
	D3DGenericBuffer m_oBVHScratch; // "Bottom Level Acceleration Stucture" but for updating ?
	D3D12_RAYTRACING_GEOMETRY_DESC m_oBVHGeometry = {};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS m_oBVHInput = {};

	D3DHitShader* m_pHitShader = nullptr;

	/*
	* Iniitialize a mesh with a single triangle
	*/
	void InitializeDebug(ID3D12Device5* a_pDevice, bool a_bUsesRayTracing = false);

	/*
	* Iniitialize a mesh from model in path
	*/
	void Initialize(std::string const a_sPath, ID3D12Device5* a_pDevice, bool a_bUsesRayTracing = false);
	void Draw(ID3D12GraphicsCommandList* a_pCommandList);

	void SetPosition(const GamePosition& a_rPosition);
	GamePosition GetPosition();
	void SetScale(const GameScale& a_rScale);
	GameScale GetScale();

	bool m_bIsIndexed = false;

	static std::vector<D3DMesh> s_MeshList;
};

