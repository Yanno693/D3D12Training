#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);
StructuredBuffer<Vertex> VertexBuffer : register(t2);
StructuredBuffer<uint16_t> IndexBuffer : register(t3);

[shader("closesthit")]
void basicsolidrt2_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    // Get Indexes from primitive ID
    const uint16_t id0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
    const uint16_t id1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
    const uint16_t id2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

    // Get Vertices
    const float3 A = VertexBuffer[id0].position;
    const float3 B = VertexBuffer[id1].position;
    const float3 C = VertexBuffer[id2].position;

    float3 AB = normalize(B - A);
    float3 AC = normalize(C - A);
    float3 normal = normalize(cross(AB, AC));
    
    float f = GetHardShadowOcclusion(scene, normal);

    payload.color = float4(0,1,0.5,0) * (1.0f - f);
}