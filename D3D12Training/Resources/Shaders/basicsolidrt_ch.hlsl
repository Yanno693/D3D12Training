#include "shader_common.hlsl"
#include "shader_common_texture.hlsl"

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

    const float2 UVA = VertexBuffer[id0].uv;
    const float2 UVB = VertexBuffer[id1].uv;
    const float2 UVC = VertexBuffer[id2].uv;

    
    float3 AB = normalize(B - A);
    float3 AC = normalize(C - A);
    float3 normal = NA * barycentrics.x + NB * barycentrics.y + NC * barycentrics.z;
    float2 uv = UVA * barycentrics.x + UVB * barycentrics.y + UVC * barycentrics.z;

    float f = GetHardShadowOcclusion(scene, normal);

    //float3 lightDirection = normalize(float3(1, 1, 1)) * - 1.0f; // TODO : Get light direction and color from constant buffer
    float3 lightDirection = normalize(oDirectionalLight.angle); // TODO : Get light direction and color from constant buffer

    float diffuseFactor = max(0.0f, dot(-lightDirection, normal));

    //float3 hitColor = A * barycentrics.x + B * barycentrics.y + C * barycentrics.z;
    //float3 hitColor = normal;

    float3 DirectionnalContribution = float3(1,1,1) * float3(oDirectionalLight.color) * (1.0f - f) * diffuseFactor;

    payload.color = float4(DirectionnalContribution, 0);

    float3 worldPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

    for(int i = 0; i < uiPointLightCount; i++)
    {
        float distanceToLight = distance(worldPos, PointLights[i].position);

        if(distanceToLight <= PointLights[i].radius)
        {
            float pointLightOcclusion = 1.0f - GetPointLightOcclusion(scene, i, distanceToLight, normal);

            float3 PosToLight = normalize(PointLights[i].position - worldPos);
            float PosToLightAngleAttenuation = max(0.0f, dot(normal, PosToLight));
            float PosToLightDistanceAttenuation = max(0.0f, 1.0f - (distanceToLight / PointLights[i].radius));

            float3 pointLightContribution = float3(1,1,1) * PointLights[i].color * PosToLightAngleAttenuation * PosToLightDistanceAttenuation * pointLightOcclusion;
        
            payload.color += float4(pointLightContribution, 0);
            //payload.color = Albedo.SampleLevel(LinearSampler, uv, 0);
            //payload.color +=  Albedo.Load(0);
        }
    }
    //payload.color = float4(normal, 0) * (1.0f - f) * diffuseFactor;
    //payload.color = float4(normal, 0);
    payload.color *= Albedo.SampleLevel(LinearSampler, uv * 10, 0);
}