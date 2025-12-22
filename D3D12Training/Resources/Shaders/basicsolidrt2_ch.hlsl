#include "shader_common.hlsl"
#include "shader_common_ch.hlsl"
#include "shader_common_texture.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);
StructuredBuffer<Vertex> VertexBuffer : register(t2);
StructuredBuffer<Index> IndexBuffer : register(t3);

/*
#ifdef USE_16BIT_INDEX_BUFFER
void basicsolidrt2_16bit_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
#else
void basicsolidrt2_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
#endif // 16_BIT_INDEX
*/

[shader("closesthit")]
RT_SHADER_SIGNATURE(basicsolidrt2)
{
    // Get Indexes from primitive ID
    const Index id0 = IndexBuffer[PrimitiveIndex() * 3 + 0];
    const Index id1 = IndexBuffer[PrimitiveIndex() * 3 + 1];
    const Index id2 = IndexBuffer[PrimitiveIndex() * 3 + 2];

    // Get Vertices
    const float3 A = VertexBuffer[id0].position;
    const float3 B = VertexBuffer[id1].position;
    const float3 C = VertexBuffer[id2].position;

    float3 AB = normalize(B - A);
    float3 AC = normalize(C - A);
    float3 normal = normalize(cross(AB, AC));
    
    float f = GetHardShadowOcclusion(scene, normal);

    //payload.color = float4(0,1,0.5,0) * (1.0f - f);
    payload.color = Albedo.SampleLevel(LinearSampler, 0, 0);
}