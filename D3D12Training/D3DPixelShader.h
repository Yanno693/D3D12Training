#pragma once

#include "D3DIncludes.h"

class D3DPixelShader
{
public:
	char* m_pByteCode; // Pointer to the begining of the bytecode of the shader
	UINT64 m_uiByteCodeSize; // Size of the bytecode
};

