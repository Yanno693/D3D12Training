#include "GameMaterial.h"

void GameMaterial::SetTexture(UINT a_uiIndex, D3DTexture* a_pTexture)
{
	m_oTextures[a_uiIndex] = a_pTexture;
}

D3DTexture* GameMaterial::GetTexture(UINT a_uiIndex)
{
	// All these shenanigans are for avoiding inserting in the map when searching
	auto pTexture = m_oTextures.find(a_uiIndex);
	
	if (pTexture != m_oTextures.end())
		return pTexture->second;
	else
		return nullptr;
}
