#pragma once

#include "D3DIncludes.h"

class D3DVertexShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	char* m_pReflectionData; // Pointer to the reflection data
	UINT64 m_uiByteCodeSize; // Size of the bytecode
	UINT64 m_uiReflectionDataSize; // Size of the reflection data
};

