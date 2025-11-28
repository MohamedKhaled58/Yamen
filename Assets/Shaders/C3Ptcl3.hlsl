// C3Ptcl3 Shader - Advanced particle rendering with color modulation
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for advanced particle effects

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
    float4 c3_VertexColor : COLOR;
    float2 c3_TexCoord0 : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR;
};

//=============================================================================
// Vertex Shader
//=============================================================================

PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Full MVP transformation
    output.position = mul(c3_ModelViewProj, input.c3_Vertex);
    
    // Pass through texture coordinates and vertex color
    output.texCoord = input.c3_TexCoord0;
    output.color = input.c3_VertexColor;
    
    return output;
}

//=============================================================================
// Pixel Shader
//=============================================================================

Texture2D Tex0 : register(t0);
SamplerState sampler0 : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = Tex0.Sample(sampler0, input.texCoord);
    return texColor * input.color;
}
