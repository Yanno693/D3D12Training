#pragma once

#include "D3DIncludes.h"
#include "D3DGenericBuffer.h"
#include "D3DRenderTarget.h"
#include "D3DMesh.h"
#include "D3DBufferManager.h"

class D3DRayTracingScene
{

private:
	std::vector<D3DMesh*> m_apCurrentSceneMesh; // Binded mesh for render
	ID3D12Device5* m_pDevice = nullptr;
	bool m_bInitialized;
	D3DTexture* m_pRenderTarget = nullptr; // To refactor i guess, a UAV to draw
 
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiBVH_CPUHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_uiBVH_GPUHandle = {};

	D3DGenericBuffer m_oBVH; // TLAS BVH for the whole scene
	D3DGenericBuffer m_oBVHScratch; // Scratch memory for generating the scene BVH
	D3DGenericBuffer m_oInstanceUpdateBuffer; // Instance Buffer, holding data for each mesh to draw

	D3DGenericBuffer m_oSceneShaderIDBuffer;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRayTracingRootSignature;
	Microsoft::WRL::ComPtr<ID3D12StateObject> m_pRayTracingPSO;


	D3DRayGenerationShader* m_oSceneRGShader = nullptr;
	D3DMissShader* m_oSceneMissShader = nullptr;
	D3DMissShader* m_oSceneMissShader2 = nullptr;

	void CreateBVH(ID3D12GraphicsCommandList4* a_pCommandList);
	void CreateShaderIDBuffer(); // Create the Shader ID Buffer for the frame, the buffer is released in ReleaseBVH()
	void ReleaseBVH();

	void LoadSceneDefaultShaders(); // To only call once, load the default Ray gen and Miss Shader for the scene
	void CreateGlobalRayTracingRootSignature(ID3D12Device5* a_pDevice); // To only call once
	void CreateGlobalRayTracingPSO(ID3D12Device5* a_pDevice);

public:

	void Initialize(ID3D12Device5* a_pDevice);
	void setRenderTarget(D3DTexture* a_pTexture);
	void SubmitForDraw(D3DMesh* a_pMesh); // Register a mesh to draw for the frame (culling could be made with this function ?)
	void DrawScene(ID3D12GraphicsCommandList4* a_pCommandList);
	void FlushRTScene();

	ID3D12RootSignature* GetGlobalRayTracingRootSignature() const;

};

extern D3DRayTracingScene g_D3DRayTracingScene;