#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("miss")]
void default2_miss(inout RTPayload payload)
{
    payload.color = float4(1,1,1,0);
}