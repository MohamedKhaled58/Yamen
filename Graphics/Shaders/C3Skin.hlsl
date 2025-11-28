// C3Skin Shader - Skeletal animation with dual bone blending
// Converted from OpenGL GLSL to DirectX 11 HLSL
// Original C3 engine shader for animated character models

//=============================================================================
// Constant Buffers
//=============================================================================

cbuffer CBPerObject : register(b0)
{
    float4x4 c3_ModelViewProj;
    float2 c3_UVAnimStep;
    float2 _padding0;
};

cbuffer CBBoneMatrices : register(b1)
{
    // 70 bones * 3 vec4 per bone = 210 vec4
    // Each bone uses 3 vec4 to represent a 3x4 transformation matrix
    float4 c3_BoneMatrix[210];
};

//=============================================================================
// Vertex Shader Input/Output
//=============================================================================

struct VSInput
{
    float3 c3_Vertex : POSITION;
    float4 c3_VertexColor : COLOR;
    float2 c3_TexCoord0 : TEXCOORD0;
    float4 c3_BoneIndexWeight : TEXCOORD1; // xy=bone indices, zw=weights (bytes)
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
    
    // Extract bone indices (multiply by 3 for matrix offset)
    int nIndex1 = int(input.c3_BoneIndexWeight.x * 3.0);
    int nIndex2 = int(input.c3_BoneIndexWeight.y * 3.0);
    
    // Extract bone weights (convert from byte format: 0-255 -> 0.0-1.0)
    float fWeight1 = input.c3_BoneIndexWeight.z * 0.0039215686; // 1/255
    float fWeight2 = input.c3_BoneIndexWeight.w * 0.0039215686; // 1/255
    
    // First bone transformation
    float4 v1 = c3_BoneMatrix[nIndex1];
    float4 v2 = c3_BoneMatrix[nIndex1 + 1];
    float4 v3 = c3_BoneMatrix[nIndex1 + 2];
    
    // Transform vertex by first bone matrix (3x4 matrix)
    float3 vVertex1;
    vVertex1.x = v1.x * input.c3_Vertex.x + v1.y * input.c3_Vertex.y + v1.z * input.c3_Vertex.z + v1.w;
    vVertex1.y = v2.x * input.c3_Vertex.x + v2.y * input.c3_Vertex.y + v2.z * input.c3_Vertex.z + v2.w;
    vVertex1.z = v3.x * input.c3_Vertex.x + v3.y * input.c3_Vertex.y + v3.z * input.c3_Vertex.z + v3.w;
    
    // Second bone transformation
    v1 = c3_BoneMatrix[nIndex2];
    v2 = c3_BoneMatrix[nIndex2 + 1];
    v3 = c3_BoneMatrix[nIndex2 + 2];
    
    // Transform vertex by second bone matrix (3x4 matrix)
    float3 vVertex2;
    vVertex2.x = v1.x * input.c3_Vertex.x + v1.y * input.c3_Vertex.y + v1.z * input.c3_Vertex.z + v1.w;
    vVertex2.y = v2.x * input.c3_Vertex.x + v2.y * input.c3_Vertex.y + v2.z * input.c3_Vertex.z + v2.w;
    vVertex2.z = v3.x * input.c3_Vertex.x + v3.y * input.c3_Vertex.y + v3.z * input.c3_Vertex.z + v3.w;
    
    // Blend the two bone transformations
    float4 vBlendVertex = float4(vVertex1 * fWeight1 + vVertex2 * fWeight2, 1.0);
    
    // Transform to clip space
    output.position = mul(c3_ModelViewProj, vBlendVertex);
    
    // Apply UV animation offset
    output.texCoord = input.c3_TexCoord0 + c3_UVAnimStep;
    
    // Pass through vertex color
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
