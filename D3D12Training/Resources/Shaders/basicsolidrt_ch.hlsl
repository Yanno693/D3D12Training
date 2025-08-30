#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("closesthit")]
void basicsolidrt_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    float f = GetHardShadowOcclusion(scene);
    
    payload.color = float4(0,0,1,0) * (1.0f - f);
}