#include "shader_common.hlsl"

struct VertexInput
{
    float3 position : POSITION;
    float3 normal : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VertexOutput main(VertexInput input)
{
    struct VertexOutput v;
    //v.position = float4(input.position.xy, 0, 1);
    //v.position = mul(ViewProjMatrix, float4(input.position, 1));

    float4 ViewSpacePos = mul(ModelMatrix, float4(input.position, 1));

    v.position = mul(ViewProjMatrix, ViewSpacePos);
    v.normal = input.normal;
    //v.position = mul(float4(input.position, 1), ViewProjMatrix);
    //v.position /= v.position.w;
    return v;
}