#include "shader_common.hlsl"

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

struct PixelOutput
{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexOutput v)
{
   
   PixelOutput p;

   p.color = float4(v.normal, 0);
   return p;
}