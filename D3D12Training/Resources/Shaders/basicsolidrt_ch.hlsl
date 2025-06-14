#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("closesthit")]
void mainHit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.color = float4(0,0,1,0);
}