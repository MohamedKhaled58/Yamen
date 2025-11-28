// Basic3D.hlsl - Standard 3D shader with lighting

// Constant Buffers
cbuffer PerFrame : register(b0)
{
    float4x4 ViewProjection;
    float3 CameraPosition;
    float _pad0;
};

cbuffer PerObject : register(b1)
{
    float4x4 World;
    float4 MaterialColor;
};

cbuffer Lighting : register(b2)
{
    float3 LightDirection;
    float _pad1;
    float3 LightColor;
    float LightIntensity;
    float3 AmbientColor;
    float _pad2;
};

// Input/Output structures
struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // Transform position to world space
    float4 worldPos = mul(float4(input.position, 1.0f), World);
    output.worldPos = worldPos.xyz;
    
    // Transform position to clip space
    output.position = mul(worldPos, ViewProjection);
    
    // Transform normal to world space (assuming uniform scaling)
    output.normal = normalize(mul(input.normal, (float3x3)World));
    
    output.texCoord = input.texCoord;
    
    return output;
}

// Pixel Shader
Texture2D Tex0 : register(t0);
SamplerState Sampler0 : register(s0);

float4 PSMain(PSInput input) : SV_TARGET
{
    // Sample texture
    float4 texColor = Tex0.Sample(Sampler0, input.texCoord);
    
    // Ambient lighting
    float3 ambient = AmbientColor * texColor.rgb;
    
    // Diffuse lighting
    float3 norm = normalize(input.normal);
    float3 lightDir = normalize(-LightDirection);
    float diff = max(dot(norm, lightDir), 0.0f);
    float3 diffuse = diff * LightColor * LightIntensity * texColor.rgb;
    
    // Specular lighting (simple Blinn-Phong)
    float3 viewDir = normalize(CameraPosition - input.worldPos);
    float3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0f), 32.0f);
    float3 specular = spec * LightColor * LightIntensity * 0.5f; // 0.5 specular strength
    
    // Combine
    float3 finalColor = (ambient + diffuse + specular) * MaterialColor.rgb;
    
    return float4(finalColor, texColor.a * MaterialColor.a);
}
