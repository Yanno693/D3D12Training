#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("miss")]
void occlusion_miss(inout OcclusionPayload payload)
{
    payload.iIsOccluded = 0;
}