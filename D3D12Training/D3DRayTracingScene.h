#pragma once

#include "D3DIncludes.h"
#include "D3DGenericBuffer.h"
#include "D3DRenderTarget.h"
#include "D3DMesh.h"
#include "D3DBufferManager.h"

class D3DRayTracingScene
{

private:
	std::vector<D3DMesh*> m_apCurrentSceneMesh;
	ID3D12Device5* m_pDevice = nullptr;
	bool m_bInitialized;
	D3DTexture* m_pRenderTarget = nullptr;
 
	D3D12_CPU_DESCRIPTOR_HANDLE m_uiBVH_CPUHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE m_uiBVH_GPUHandle = {};

	D3DGenericBuffer m_oBVH;
	D3DGenericBuffer m_oBVHScratch;
	D3DGenericBuffer m_oInstanceUpdateBuffer;

	D3DRayGenerationShader* m_oSceneRGShader = nullptr;

	void CreateBVH(ID3D12GraphicsCommandList4* a_pCommandList);
	void ReleaseBVH();

public:

	void Initialize(ID3D12Device5* a_pDevice);
	void setRenderTarget(D3DTexture* a_pTexture);
	void SubmitForDraw(D3DMesh* a_pMesh);
	void DrawScene(ID3D12GraphicsCommandList4* a_pCommandList);
	void flushSceneMesh();

};

extern D3DRayTracingScene g_D3DRayTracingScene;