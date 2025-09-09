#pragma once

#include <DirectXMath.h>

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

struct GameCamera
{
	GameTransform transform;
};

struct GameScreenResolution
{
	UINT width, height;
};

struct _ObjectData
{
	DirectX::XMMATRIX oModelMatrix;
};