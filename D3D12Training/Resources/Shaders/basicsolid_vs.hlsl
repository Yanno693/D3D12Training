#include "shader_common.hlsl"

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

VertexOutput main(Vertex input)
{
    struct VertexOutput v;

    float4 ViewSpacePos = mul(ModelMatrix, float4(input.position, 1));

    v.position = mul(oViewProjMatrix, ViewSpacePos);
    v.normal = input.normal;
    //v.position /= v.position.w; // Semms like i don't need this line ?
    
    return v;
}