// C3PuzzleCell2 Shader - Puzzle game cell rendering (type 2)
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Identical to PuzzleCell1 but for different cell types

//=============================================================================
// Constant Buffers
//=============================================================================

cbuffer CBPerFrame : register(b0)
{
    float2 c3_PixelSize;
    float2x2 c3_RotateImageMatrix;
};

//=============================================================================
// Vertex Shader Input/Output
//=============================================================================

struct VSInput
{
    float2 c3_Vertex : POSITION;
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
    
    float2 vProjPos;
    vProjPos.x = input.c3_Vertex.x * c3_PixelSize.x - 1.0;
    vProjPos.y = input.c3_Vertex.y * c3_PixelSize.y + 1.0;
    
    float2 rotated = mul(c3_RotateImageMatrix, vProjPos);
    
    output.position = float4(rotated, 0.0, 1.0);
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
    
    float3 vFinalRGB = texColor.rgb * input.color.rgb;
    float fFinalAlpha = texColor.a * input.color.a;
    
    return float4(vFinalRGB, fFinalAlpha);
}
