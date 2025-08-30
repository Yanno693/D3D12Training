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
	char* m_pByteCode = nullptr;
	UINT64 m_uiByteCodeSize = 0;
	std::string m_szShaderIdentifier = "";
};

class D3DRayGenerationShader : public D3DRTShader
{
	// Only one RayGenerationShader for the whole scene ... normally
public:
	static UINT g_ShaderCount;

	D3DRayGenerationShader()
	{
		g_ShaderCount++;
	}
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
* Ray Generation :
* 0 : Scene Ray Generation Shader
* 
* Miss table shaders :
* 0 : AO miss (for AO for example) // TODO : Make AO Shaders
* 1 : Skybox
* 
* Hit table Shaders :
* For Each Mesh :
*	0 : Mesh Albedo Shader
*	1 : AO Hit Shader
*/
