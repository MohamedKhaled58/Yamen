// C3Font Shader - Text rendering with alpha-based coloring
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for bitmap font rendering

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
    float4 c3_Vertex : POSITION;      // Position in pixel coordinates (uses vec4 in original)
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
    
    // Convert pixel coordinates to NDC (Normalized Device Coordinates)
    float2 vProjPos;
    vProjPos.x = input.c3_Vertex.x * c3_PixelSize.x - 1.0;
    vProjPos.y = input.c3_Vertex.y * c3_PixelSize.y + 1.0;
    
    // Apply rotation matrix
    float2 rotated = mul(c3_RotateImageMatrix, vProjPos);
    
    // Output position (z=0, w=1 for 2D rendering)
    output.position = float4(rotated, 0.0, 1.0);
    
    // Pass through texture coordinates and color
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
    
    // Font rendering: Use vertex color RGB, but alpha from texture
    // This allows changing text color while preserving the font's alpha mask
    return float4(input.color.xyz, texColor.w);
}
