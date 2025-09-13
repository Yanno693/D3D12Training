#pragma once
#include "ShaderShared.h"
#include "D3DConstantBuffer.h"
#include "D3DGenericBuffer.h"

class GameScene
{
private:
	std::vector<GamePointLight> m_aPointLights;
	GameDirectionalLight m_oDirectionalLight;


public:
	D3DConstantBuffer m_pSceneConstantBuffer; // Scene Constant buffer
	D3DGenericBuffer m_pPointLightBuffer; // Structured buffer of light
	GameSceneData m_oSceneData;

	void Initialize();

	UINT GetPointLightsCount() const;
	GameDirectionalLight& GetDirectionalLight();
	void UploadPointLightsToGPU();

	void AddPointLight(const GamePointLight& a_oPointLight);

};

extern GameScene g_GameScene;
extern GameScreenResolution g_ScreenResolution;

