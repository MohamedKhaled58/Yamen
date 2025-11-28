// C3MixSprite Shader - Mixed sprite rendering
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for mixed/blended sprite rendering

//=============================================================================
// Constant Buffers
//=============================================================================

cbuffer CBPerFrame : register(b0)
{
    float2 c3_PixelSize;        // (2/screenWidth, -2/screenHeight)
    float2x2 c3_RotateImageMatrix; // 2x2 rotation matrix
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
    
    // Convert pixel coordinates to NDC
    float2 vProjPos;
    vProjPos.x = input.c3_Vertex.x * c3_PixelSize.x - 1.0;
    vProjPos.y = input.c3_Vertex.y * c3_PixelSize.y + 1.0;
    
    // Apply rotation matrix
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
    return texColor * input.color;
}
