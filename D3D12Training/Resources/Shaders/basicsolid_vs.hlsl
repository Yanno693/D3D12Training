#include "shader_common.hlsl"

struct VertexInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput main(VertexInput input)
{
    struct VertexOutput v;
    //v.position = float4(input.position.xy, 0, 1);
    v.position = mul(ViewProjMatrix, float4(input.position, 1));
    //v.position /= v.position.w;
    return v;
}