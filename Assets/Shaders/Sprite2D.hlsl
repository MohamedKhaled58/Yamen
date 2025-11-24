// 2D Sprite Shader
cbuffer PerFrame : register(b0)
{
    matrix ViewProjection;
};

struct VSInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    return texColor * input.Color;
}
