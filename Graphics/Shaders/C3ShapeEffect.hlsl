// C3ShapeEffect Shader - Shape effect rendering
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for special shape effects

//=============================================================================
// Constant Buffers
//=============================================================================

cbuffer CBPerObject : register(b0)
{
    float4x4 c3_ModelViewProj;
};

//=============================================================================
// Vertex Shader Input/Output
//=============================================================================

struct VSInput
{
    float4 c3_Vertex : POSITION;
    float2 c3_TexCoord0 : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

//=============================================================================
// Vertex Shader
//=============================================================================

PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Simple MVP transformation
    output.position = mul(c3_ModelViewProj, input.c3_Vertex);
    output.texCoord = input.c3_TexCoord0;
    
    return output;
}

//=============================================================================
// Pixel Shader
//=============================================================================

Texture2D Tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    // Simple textured rendering
    return Tex0.Sample(sampler0, input.texCoord);
}
