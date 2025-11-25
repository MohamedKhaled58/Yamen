// C3Sprite.hlsl - Port of original Conquer Online 2D sprite shader
// Handles 2D sprites with rotation and vertex colors

cbuffer PerFrame : register(b0)
{
    float2 PixelSize;     // Screen pixel size for NDC conversion
    float2x2 RotateMatrix; // 2D rotation matrix
};

struct VSInput
{
    float2 Position  : POSITION;
    float4 Color     : COLOR;
    float2 TexCoord  : TEXCOORD0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR;
    float2 TexCoord : TEXCOORD0;
};

Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Convert to NDC (-1 to 1)
    float2 projPos = float2(
        input.Position.x * PixelSize.x - 1.0,
        input.Position.y * PixelSize.y + 1.0
    );
    
    // Apply rotation
    float2 rotated = mul(RotateMatrix, projPos);
    output.Position = float4(rotated, 0.0, 1.0);
    
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    return texColor * input.Color;
}
