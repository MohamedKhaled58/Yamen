// Sprite2D.hlsl - Standard 2D sprite shader

// Constant Buffers
cbuffer PerFrame : register(b0)
{
    float4x4 ViewProjection;
};

// Input/Output structures
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 texCoord : TEXCOORD0;
};

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Transform position to clip space
    // Note: SpriteBatch handles World transform on CPU, so we only need ViewProjection
    output.position = mul(ViewProjection, float4(input.position, 1.0f));
    
    output.color = input.color;
    output.texCoord = input.texCoord;
    
    return output;
}

// Pixel Shader
Texture2D Tex0 : register(t0);
SamplerState Sampler0 : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    // Sample texture and multiply by vertex color
    return Tex0.Sample(Sampler0, input.texCoord) * input.color;
}
