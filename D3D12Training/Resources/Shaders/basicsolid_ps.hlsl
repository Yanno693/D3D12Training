#include "shader_common.hlsl"
#include "shader_common_texture.hlsl"

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexOutput v)
{
   PixelOutput p;

   //p.color = float4(v.normal, 0);
   p.color = Albedo.Sample(LinearSampler, v.uv);
   return p;
}