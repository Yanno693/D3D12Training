#pragma once

#include "D3DIncludes.h"

class D3DRayGenerationShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	UINT64 m_uiByteCodeSize; // Size of the bytecode
};

class D3DHitShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	UINT64 m_uiByteCodeSize; // Size of the bytecode
	UINT m_uiShaderTableIndex = UINT_MAX; // ID of the shader in the Hit Group
};

class D3DMissShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	UINT64 m_uiByteCodeSize; // Size of the bytecode
	UINT m_uiShaderTableIndex = UINT_MAX;  // ID of the shader in the Miss Group (miss group ?)
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
