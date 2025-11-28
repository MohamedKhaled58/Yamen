// C3Ptcl1 Shader - Simple particle rendering
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for basic particle effects

//=============================================================================
// Constant Buffers
//=============================================================================

cbuffer CBPerFrame : register(b0)
{
    float4x4 c3_Proj; // Projection matrix only (no model/view)
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
    
    // Project using projection matrix only
    output.position = mul(c3_Proj, input.c3_Vertex);
    
    // Pass through texture coordinates
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
    // Simple textured rendering without color modulation
    return Tex0.Sample(sampler0, input.texCoord);
}
