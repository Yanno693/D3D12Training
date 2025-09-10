#pragma once
#include "ShaderShared.h"
#include "D3DConstantBuffer.h"
#include "D3DGenericBuffer.h"


class GameScene
{
public:
	D3DConstantBuffer m_pSceneConstantBuffer; // Scene Constant buffer
	D3DGenericBuffer m_pLightBuffer; // Structured buffer of light
};

extern GameScene g_GameScene;
extern GameScreenResolution g_ScreenResolution;

