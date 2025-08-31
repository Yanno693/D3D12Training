#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("closesthit")]
void occlusion_hit(inout OcclusionPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    payload.iIsOccluded = 1;
}