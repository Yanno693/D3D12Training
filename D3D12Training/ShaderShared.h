#pragma once

#ifdef __cplusplus
	#include "GameIncludes.h"
	#include "D3DIncludes.h"

	#define cbuffer struct
	#define float4x4 DirectX::XMMATRIX
	#define float2 DirectX::XMFLOAT2
	#define REGISTER(_bind, _space)
	#define uint UINT
#else // HLSL
	#define GamePosition float3
	#define GameColor float3
	#define GameRotation float3
	#define REGISTER(_bind, _space) : register(_bind, _space)
#endif

#define MAX_POINTLIGHT_COUNT 100

struct GamePointLight
{
	GamePosition position;
	GameColor color;
	float radius;
};

struct GameDirectionalLight
{
	GameColor color;
	GameRotation angle;
};

// Constant buffer packing rules : https://maraneshi.github.io/HLSL-ConstantBufferLayoutVisualizer/
cbuffer GameSceneData REGISTER(b0, space0)
{
	float4x4 oViewProjMatrix;
	float4x4 oInvProjMatrix;
	float4x4 oInvViewMatrix;
	GamePosition oCameraWorldPos;
	uint uiPointLightCount;
	float2 oScreenSize;
	GameDirectionalLight oDirectionalLight;
};