#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);
StructuredBuffer<Vertex> VertexBuffer : register(t2);
StructuredBuffer<uint16_t> IndexBuffer : register(t3);

[shader("closesthit")]
void basicsolidrt_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    
    float3 barycentrics = float3(1.f - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    // Get Indexes from primitive ID
    const uint16_t id0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
    const uint16_t id1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
    const uint16_t id2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

    // Get Vertices
    const float3 A = VertexBuffer[id0].position;
    const float3 B = VertexBuffer[id1].position;
    const float3 C = VertexBuffer[id2].position;

    const float3 NA = VertexBuffer[id0].normal;
    const float3 NB = VertexBuffer[id1].normal;
    const float3 NC = VertexBuffer[id2].normal;

    
    float3 AB = normalize(B - A);
    float3 AC = normalize(C - A);
    float3 normal = NA * barycentrics.x + NB * barycentrics.y + NC * barycentrics.z;

    float f = GetHardShadowOcclusion(scene, normal);

    //float3 hitColor = A * barycentrics.x + B * barycentrics.y + C * barycentrics.z;
    //float3 hitColor = normal;

    //payload.color = float4(0,0,1,0) * (1.0f - f);
    payload.color = float4(normal, 0) * (1.0f - f);
    //payload.color = float4(normal, 0);
}