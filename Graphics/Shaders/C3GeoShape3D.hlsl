// C3GeoShape3D Shader - 3D geometric shape rendering
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for 3D shapes without textures

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
    float3 c3_Vertex : POSITION;
    float4 c3_VertexColor : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

//=============================================================================
// Vertex Shader
//=============================================================================

PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Simple MVP transformation
    output.position = mul(c3_ModelViewProj, float4(input.c3_Vertex, 1.0));
    output.color = input.c3_VertexColor;
    
    return output;
}

//=============================================================================
// Pixel Shader
//=============================================================================

float4 PSMain(PSInput input) : SV_TARGET
{
    // Solid color from vertex color (no texture)
    return input.color;
}
