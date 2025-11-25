// C3PhyMesh.hlsl - Port of original Conquer Online static mesh shader
// Handles static 3D meshes with vertex colors

cbuffer PerFrame : register(b0)
{
    matrix ModelViewProj;
};

struct VSInput
{
    float4 Position  : POSITION;
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
    
    // Transform to clip space (already column-major, no transpose needed)
    output.Position = mul(input.Position, ModelViewProj);
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
