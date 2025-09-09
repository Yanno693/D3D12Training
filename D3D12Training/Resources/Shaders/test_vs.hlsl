struct VertexInput
{
    float2 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
};

#include "shader_common.hlsl"

VertexOutput main(VertexInput input)
{
    struct VertexOutput v;
    v.position = float4(input.position + oViewProjMatrix[0][0], 0, 1);
    return v;
}