Texture2D<float4> Albedo : register(t10);
Texture2D<float4> Normal : register(t11);
SamplerState LinearSampler : register(s0);


float3 SampleAlbedo(float2 uv)
{
    return Albedo.Sample(LinearSampler, uv).xyz;
}

float3 SampleAlbedo(float2 uv, uint mipLevel)
{
    return Albedo.SampleLevel(LinearSampler, uv, mipLevel).xyz;
}

float3 SampleNormal(float2 uv)
{
    // Work only on signed normal
    float2 normalXY = Normal.Sample(LinearSampler, uv).xy;
    float normalZ = sqrt(1.0f - normalXY.x * normalXY.x + normalXY.y * normalXY.y);
    return float3(normalXY, normalZ);
}

float3 SampleNormal(float2 uv, uint mipLevel)
{
    // Work only on signed normal
    float2 normalXY = Normal.SampleLevel(LinearSampler, uv, mipLevel).xy;
    float normalZ = sqrt(1.0f - normalXY.x * normalXY.x + normalXY.y * normalXY.y);
    return float3(normalXY, normalZ);
}