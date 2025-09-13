#include "GameScene.h"
#include "D3DBufferManager.h"

GameScene g_GameScene;
GameScreenResolution g_ScreenResolution;

void GameScene::Initialize()
{
	g_D3DBufferManager.InitializeGenericBuffer(&m_pPointLightBuffer, sizeof(GamePointLight) * MAX_POINTLIGHT_COUNT);
	m_pPointLightBuffer.SetDebugName(L"Scene Point Light Structured Buffer");
}

UINT GameScene::GetPointLightsCount() const
{
	return m_aPointLights.size();
}

GameDirectionalLight& GameScene::GetDirectionalLight()
{
	return m_oDirectionalLight;
}

void GameScene::UploadPointLightsToGPU()
{
	m_pPointLightBuffer.WriteData(m_aPointLights.data(), m_aPointLights.size() * sizeof(GamePointLight));
}

void GameScene::AddPointLight(const GamePointLight& a_oPointLight)
{
	m_aPointLights.push_back(a_oPointLight);
}
