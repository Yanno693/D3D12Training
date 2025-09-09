#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

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
    
    uav[out_idx] = payLoad.color;
}