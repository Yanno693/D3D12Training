#pragma once

#include "D3DIncludes.h"

enum D3D_RT_SHADER_TYPE
{
	RAYGEN,
	HIT,
	MISS
};

class D3DRTShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	UINT64 m_uiByteCodeSize; // Size of the bytecode
};

class D3DRayGenerationShader : public D3DRTShader
{
	// Only one RayGenerationShader for the whole scene ... normally
};

class D3DHitShader : public D3DRTShader
{
public:
	UINT m_uiShaderTableIndex = UINT_MAX; // ID of the shader in the Hit Group
	static UINT g_ShaderCount;

	D3DHitShader()
	{
		m_uiShaderTableIndex = g_ShaderCount;
		g_ShaderCount++;
	}
};

class D3DMissShader : public D3DRTShader
{
public:
	UINT m_uiShaderTableIndex = UINT_MAX;  // ID of the shader in the Miss Table
	static UINT g_ShaderCount;

	D3DMissShader()
	{
		m_uiShaderTableIndex = g_ShaderCount;
		g_ShaderCount++;
	}
};

/*
* Shader Organization :
* 
* ___________
* Ray Generation
* Miss Shaders
* Hit Shaders
* ___________
* 
* Miss shaders :
* 0 : Skybox
* 1 : Hit or miss (for AO for example)
* 
* Hit Shaders :
* 0 : Basic RT
* 1 : HiT or Miss
*/
