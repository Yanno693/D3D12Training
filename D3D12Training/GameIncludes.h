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

struct _SceneData
{
	DirectX::XMFLOAT2 oScreenSize;
	DirectX::XMMATRIX oViewProjMatrix;
	DirectX::XMMATRIX oInvProjMatrix;
	DirectX::XMMATRIX oInvViewMatrix;
};

struct _ObjectData
{
	DirectX::XMMATRIX oModelMatrix;
};

struct GamePosition
{
	float x, y, z;
};

struct GameRotation
{
	float x, y, z;
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

struct GameCamera
{
	GameTransform transform;
};

struct GameScreenResolution
{
	UINT width, height;
};