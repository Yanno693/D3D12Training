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

   if(v.position.x > 0.5)
   {

   p.color = float4(1, 0, 1, 1);
   }
   else
   {
   p.color = float4(0, 0, 1, 1);

   }
   return p;
}