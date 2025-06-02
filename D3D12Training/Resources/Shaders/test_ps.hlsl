#include "shader_common.hlsl"

struct VertexOutput
{
    float4 position : SV_POSITION;
};

struct PixelOutput
{
    float4 color : SV_TARGET0;
};

PixelOutput main(VertexOutput v)
{
   
   PixelOutput p;
   p.color = float4(1, 0, 1, 1);
   p.color.y = ViewProjMatrix[0][0];
   return p;
}