#define D3DCOMPILE_DEBUG 1

cbuffer SceneData : register(b0, space0)
{
    float2 screenSize;
    float4x4 ViewProjMatrix;
    float4x4 InvProjMatrix;
    float4x4 InvViewMatrix;
};

cbuffer InstanceData : register(b0, space1)
{
    float4x4 ModelMatrix;
};

struct RTPayload
{
    float4 color;
};