#include "../../ShaderShared.h" // Shared types between C++ and HLSL

#define D3DCOMPILE_DEBUG 1

#define DEFAULT_OCCLUSION_SHADER_ID 0
#define DEFAULT_MISS_SHADER_ID 1

#define DIRECT_HIT_SHADER_OFFSET 0
#define OCCLUSION_HIT_SHADER_OFFSET 1

#define MESH_SHADER_GROUP_SIZE 2

// https://gist.github.com/keijiro/ee7bc388272548396870
float nrand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

// RT Only
struct RTPayload
{
    float4 color;
};

// RT Only
struct OcclusionPayload
{
    int iIsOccluded;
};

// Shader between Raster and RT
struct Vertex
{
    float3 position : POSITION;
    float3 normal : TEXCOORD;
};

cbuffer InstanceData : register(b1, space0)
{
    float4x4 ModelMatrix;
};

StructuredBuffer<GamePointLight> PointLights : register(t1);

float GetHardShadowOcclusion(RaytracingAccelerationStructure a_scene, float3 surface_normal)
{
    OcclusionPayload payLoad;

    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent() + surface_normal * 0.001;

    ray.Direction = -normalize(oDirectionalLight.angle); // TODO : Get directional light direction from constant buffer
    ray.TMin = 0.0001;
    ray.TMax = 1000;

    payLoad.iIsOccluded = 0;

    TraceRay(a_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES , 0xFF, OCCLUSION_HIT_SHADER_OFFSET, MESH_SHADER_GROUP_SIZE, DEFAULT_OCCLUSION_SHADER_ID, ray, payLoad);

    return payLoad.iIsOccluded;
}

float GetPointLightOcclusion(RaytracingAccelerationStructure a_scene, float PointLightIndex, float distanceToLight, float3 surface_normal)
{
    OcclusionPayload payLoad;

    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent() + surface_normal * 0.001;

    ray.Direction = normalize(PointLights[PointLightIndex].position - ray.Origin); // TODO : Get directional light direction from constant buffer
    ray.TMin = 0.0001;
    ray.TMax = distanceToLight;

    payLoad.iIsOccluded = 0;

    TraceRay(a_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES , 0xFF, OCCLUSION_HIT_SHADER_OFFSET, MESH_SHADER_GROUP_SIZE, DEFAULT_OCCLUSION_SHADER_ID, ray, payLoad);

    return payLoad.iIsOccluded;
}