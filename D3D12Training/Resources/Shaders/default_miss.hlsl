#include "shader_common.hlsl"

RaytracingAccelerationStructure scene : register(t0);
RWTexture2D<float4> uav : register(u0);

[shader("miss")]
void default_miss(inout RTPayload payload)
{
    float sunVisibility = pow(max(0.0f, dot(-normalize(oDirectionalLight.angle), WorldRayDirection())), 100.0f); 
/*
    float sunVisibility = max(0.0f, dot(-normalize(oDirectionalLight.angle), WorldRayDirection())); 

    sunVisibility = -sunVisibility + 1.0f;
    sunVisibility /= 100.0f;
    sunVisibility = -sunVisibility + 1.0f;
*/
    payload.color = float4(1,1,1,0) * sunVisibility;
}