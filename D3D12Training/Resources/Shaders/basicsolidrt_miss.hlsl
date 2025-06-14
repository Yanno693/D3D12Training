#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("miss")]
void mainMiss(inout RTPayload payload)
{
    payload.color = float4(1,0,0,0);
}