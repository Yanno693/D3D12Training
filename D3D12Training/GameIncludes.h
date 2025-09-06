#pragma once

#include <DirectXMath.h>

struct _2DVertex
{
	float x, y;
};

struct _3DVertex
{
	float x, y, z;
};

struct GamePosition
{
	float x, y, z;
};

struct GameColor
{
	float r, g, b;
};

struct GameRotation
{
	float x, y, z;
	DirectX::XMMATRIX rotationMatrix;
};

struct GameScale
{
	float x, y, z;
};

struct GameTransform
{
	GamePosition position;
	GameRotation rotation;
	GameScale scale;
};

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

struct GameCamera
{
	GameTransform transform;
};

struct GameScreenResolution
{
	UINT width, height;
};

struct _SceneData
{
	DirectX::XMFLOAT2 oScreenSize;
	DirectX::XMMATRIX oViewProjMatrix;
	DirectX::XMMATRIX oInvProjMatrix;
	DirectX::XMMATRIX oInvViewMatrix;
	GameDirectionalLight oDirectionalLight;
};

struct _ObjectData
{
	DirectX::XMMATRIX oModelMatrix;
};