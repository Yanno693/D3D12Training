#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

float3 SunVisibility(float3 ViewRayDirection)
{
    RayDesc ray;
    ray.Origin = oCameraWorldPos;
    ray.Direction = normalize(-oDirectionalLight.angle);
    ray.TMin = 0.001;
    ray.TMax = 1000;

    OcclusionPayload payLoad;
    TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES , 0xFF, OCCLUSION_HIT_SHADER_OFFSET, MESH_SHADER_GROUP_SIZE, DEFAULT_OCCLUSION_SHADER_ID, ray, payLoad);
    
    float sunVisibility = pow(max(0.0f, dot(-normalize(oDirectionalLight.angle), ViewRayDirection)), 100.0f); 

    return oDirectionalLight.color * (1.0f - payLoad.iIsOccluded) * sunVisibility;
}

float3 PointLightVisibility(float3 ViewRayDirection, int PointLightIndex)
{
    float3 viewLightDir = normalize(PointLights[PointLightIndex].position - oCameraWorldPos);
    float viewLightDistance = distance(PointLights[PointLightIndex].position, oCameraWorldPos);
    
    RayDesc ray;
    ray.Origin = oCameraWorldPos;
    ray.Direction = viewLightDir;
    ray.TMin = 0.001;
    ray.TMax = viewLightDistance;

    OcclusionPayload payLoad;
    TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES , 0xFF, OCCLUSION_HIT_SHADER_OFFSET, MESH_SHADER_GROUP_SIZE, DEFAULT_OCCLUSION_SHADER_ID, ray, payLoad);
    
    float reducedDot = max(0.0f, dot(viewLightDir, ViewRayDirection));
    //reducedDot = 1.0f - reducedDot;
    //reducedDot /= viewLightDistance;
    //reducedDot = 1.0f - reducedDot;
    float lightVisibility = pow(reducedDot, 100.0f + viewLightDistance * viewLightDistance); 

    return PointLights[PointLightIndex].color * (1.0f - payLoad.iIsOccluded) * lightVisibility / viewLightDistance;
}

[shader("raygeneration")]
void default_raygen()
{
    uint2 idx = DispatchRaysIndex().xy;
    uint3 dim = DispatchRaysDimensions();

    uint2 out_idx = int2(idx.x, dim.y - 1 - idx.y);

    float4 NDCNearPosition = float4((float2(idx) / oScreenSize) * 2.0f - 1.0f, 0, 1);
    float4 NDCFarPosition = float4((float2(idx) / oScreenSize) * 2.0f - 1.0f, 1, 1);
    float4 VSNearPosition = mul(oInvProjMatrix, NDCNearPosition);
    float4 VSFarPosition = mul(oInvProjMatrix, NDCFarPosition);
    VSNearPosition /= VSNearPosition.w;
    VSFarPosition /= VSFarPosition.w;

    float4 WSNearPosition = mul(oInvViewMatrix, VSNearPosition);
    float4 WSFarPosition = mul(oInvViewMatrix, VSFarPosition);
    WSNearPosition /= WSNearPosition.w;
    WSFarPosition /= WSFarPosition.w;

    RayDesc ray;
    ray.Origin = WSNearPosition.xyz;
    ray.Direction = normalize(WSFarPosition.xyz - WSNearPosition.xyz);
    ray.TMin = 0.001;
    ray.TMax = 1000;

    RTPayload payLoad;
    
    TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, DIRECT_HIT_SHADER_OFFSET, 2, DEFAULT_MISS_SHADER_ID, ray, payLoad);

    float3 lightsBloom = SunVisibility(ray.Direction);

    for(int i = 0; i < uiPointLightCount; i++)
    {
        lightsBloom += PointLightVisibility(ray.Direction, i);
    }

    uav[out_idx] = payLoad.color + float4(lightsBloom, 0);
}