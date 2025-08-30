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

struct OcclusionPayload
{
    int iHasTouched;
};

/*
float GetOcclusion()
{
    OcclusionPayload payloads[10];

    float occlusion = 0;

    for(int i = 0; i < 10; i++)
    {

    }
    
    return 0.0f;
}
*/

float GetHardShadowOcclusion(RaytracingAccelerationStructure a_scene)
{
    OcclusionPayload payLoad;
    //RTPayload payLoad;

    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    ray.Direction = normalize(float3(1, 1, 1)); // TODO : Get normal of the triangle or something
    ray.TMin = 0.01;
    ray.TMax = 1000;

    payLoad.iHasTouched = 0;

    TraceRay(a_scene, RAY_FLAG_NONE, 0xFF, OCCLUSION_HIT_SHADER_OFFSET, MESH_SHADER_GROUP_SIZE, DEFAULT_OCCLUSION_SHADER_ID, ray, payLoad);

    return payLoad.iHasTouched;
}