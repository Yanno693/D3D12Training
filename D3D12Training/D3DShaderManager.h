#pragma once

#include "D3DIncludes.h"
#include "D3DVertexShader.h"
#include "D3DPixelShader.h"
#include "D3DRTShader.h"

#include <unordered_map>

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
	void LoadShader(std::string const a_sPath); // Load a shader and keep it
	void LoadRTShader(std::string const a_sPath); // Load a ray tracing shader and keep it

	Microsoft::WRL::ComPtr<IDxcUtils> m_pUtils = nullptr; // DXC Utilitary for shader reflection
	
public:
	D3DShaderPair* RequestShader(std::string const a_sPath); // Return a shader
	D3DRTShaderGroup* RequestRTShader(std::string const a_sPath); // Return a ray tracing shader
};

extern D3DShaderManager g_D3DShaderManager;

