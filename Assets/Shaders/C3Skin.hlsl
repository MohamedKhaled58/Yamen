// C3Skin.hlsl - Port of original Conquer Online skinned mesh shader
// Handles character/object animation with skeletal skinning (up to 70 bones)

cbuffer PerFrame : register(b0)
{
    matrix ModelViewProj;
    float2 UVAnimStep;        // UV animation offset per frame
    float2 _pad0;
};

cbuffer BoneMatrices : register(b1)
{
    // Each bone is stored as 3 vec4s (rows of a 4x3 matrix)
    // Supports up to 70 bones (210 vec4s total)
    float4 BoneMatrix[210];
};

struct VSInput
{
    float3 Position  : POSITION;
    float4 Color     : COLOR;
    float2 TexCoord  : TEXCOORD0;
    float4 BoneIndexWeight : BLENDINDICES; // xy = bone indices, zw = weights (0-255 packed)
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
    
    // Extract bone indices (multiply by 3 to get matrix start)
    int index1 = int(input.BoneIndexWeight.x * 3.0);
    int index2 = int(input.BoneIndexWeight.y * 3.0);
    
    // Extract weights (packed as 0-255, convert to 0-1)
    float weight1 = input.BoneIndexWeight.z * 0.0039215686; // 1/255
    float weight2 = input.BoneIndexWeight.w * 0.0039215686;
    
    // Transform by first bone
    float4 row1_1 = BoneMatrix[index1];
    float4 row2_1 = BoneMatrix[index1 + 1];
    float4 row3_1 = BoneMatrix[index1 + 2];
    
    float3 vertex1 = float3(
        dot(row1_1.xyz, input.Position) + row1_1.w,
        dot(row2_1.xyz, input.Position) + row2_1.w,
        dot(row3_1.xyz, input.Position) + row3_1.w
    );
    
    // Transform by second bone
    float4 row1_2 = BoneMatrix[index2];
    float4 row2_2 = BoneMatrix[index2 + 1];
    float4 row3_2 = BoneMatrix[index2 + 2];
    
    float3 vertex2 = float3(
        dot(row1_2.xyz, input.Position) + row1_2.w,
        dot(row2_2.xyz, input.Position) + row2_2.w,
        dot(row3_2.xyz, input.Position) + row3_2.w
    );
    
    // Blend between bones
    float3 blendedVertex = vertex1 * weight1 + vertex2 * weight2;
    
    // Transform to clip space
    output.Position = mul(float4(blendedVertex, 1.0), ModelViewProj);
    
    // UV animation
    output.TexCoord = input.TexCoord + UVAnimStep;
    output.Color = input.Color;
    
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    return texColor * input.Color;
}
