#pragma once

#include "D3DIncludes.h"
#include "D3DVertexShader.h"
#include "D3DPixelShader.h"
#include "D3DRTShader.h"

#include <unordered_map>
#include <map>

class D3DShaderPair
{
public:
	D3DVertexShader* m_pVS;
	D3DPixelShader* m_pPS;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vInputElements;
	std::vector<D3D12_SIGNATURE_PARAMETER_DESC> m_vSignatureElements; // Need to be saved, to avoid issue with sementics
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> m_pReflection;
};

class D3DRTShaderGroup
{
public:
	D3DRayGenerationShader* m_pRGS;
	D3DHitShader* m_pHS;
	D3DMissShader* m_pMS;


	// TODO : Determine if we'll need this
	/*
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vInputElements;
	std::vector<D3D12_SIGNATURE_PARAMETER_DESC> m_vSignatureElements; // Need to be saved, to avoid issue with sementics
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection> m_pReflection;
	*/
};

class D3DShaderManager
{
	/*
	We will allocate D3DShaderPairs in the stack to avoid meshes
	to lose their pointers when the table changes its size,
	and store the pointer in the table directly
	*/
private:
	std::unordered_map<std::string, D3DShaderPair*> m_oShaderSet; // Table of all loaded shader. If multiple mesh need the same shader, it's in here
	std::unordered_map<std::string, D3DRTShaderGroup*> m_oRTShaderSet; // Table of all loaded shader. If multiple mesh need the same shader, it's in here
	
	// V2
	std::map<std::string, D3DRTShader*> m_oRayGenShaderSet;
	std::map<std::string, D3DRTShader*> m_oMissShaderSet;
	std::map<std::string, D3DRTShader*> m_oHitShaderSet;
	bool m_bDirtyRTPSO = true; // The PSO gets dry when a shader is added, and gets cleen when the PSO is rebuilt by the RT Scene

	void LoadShader(std::string const a_sPath); // Load a shader and keep it
	void LoadRTShader(std::string const a_sPath); // Load a ray tracing shader and keep it
	void LoadRTShader(std::string const a_sPath, D3D_RT_SHADER_TYPE const a_eShaderType); // Load a ray tracing shader and keep it

	Microsoft::WRL::ComPtr<IDxcUtils> m_pUtils = nullptr; // DXC Utilitary for shader reflection
	
public:
	D3DShaderPair* RequestShader(std::string const a_sPath); // Return a shader
	D3DRTShaderGroup* RequestRTShader(std::string const a_sPath); // Return a ray tracing shader
	D3DRTShader* RequestRTShaderV2(std::string const a_sPath, D3D_RT_SHADER_TYPE const a_eShaderType); // Return a ray tracing shader

	std::map<std::string, D3DRTShader*>& GetRTShaderSet(D3D_RT_SHADER_TYPE const a_eShaderType);

	bool IsRTPSODirty() const;
	void SetRTPSOClean();

	UINT GetRTShadersCount() const;
};

extern D3DShaderManager g_D3DShaderManager;

