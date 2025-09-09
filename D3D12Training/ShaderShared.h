#pragma once

#ifdef __cplusplus
	#include "GameIncludes.h"

	#define cbuffer struct
	#define float4x4 DirectX::XMMATRIX
	#define float2 DirectX::XMFLOAT2
	#define REGISTER(_bind, _space) 
#else // HLSL
	#define GamePosition float3
	#define GameColor float3
	#define GameRotation float3
	#define REGISTER(_bind, _space) : register(_bind, _space)
#endif

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

cbuffer GameSceneData REGISTER(b0, space0)
{
	float2 oScreenSize;
	float4x4 oViewProjMatrix;
	float4x4 oInvProjMatrix;
	float4x4 oInvViewMatrix;
	GameDirectionalLight oDirectionalLight;
};