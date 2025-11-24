#pragma once

#include "ShaderShared.h"
#include "D3DTexture.h"
#include <map>

enum GameMaterialTextureIndex
{
	Albedo,
	Normal
};

class GameMaterial
{
private:

	std::map<UINT, D3DTexture*> m_oTextures; // Texture of the material

public:
	void SetTexture(UINT a_uiIndex, D3DTexture* a_pTexture);
	D3DTexture* GetTexture(UINT a_uiIndex);
};

