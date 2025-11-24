// Basic 3D Shader with Blinn-Phong Lighting
cbuffer PerFrame : register(b0)
{
    matrix ViewProjection;
    float3 CameraPosition;
    float _pad0;
};

cbuffer PerObject : register(b1)
{
    matrix World;
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

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

Texture2D DiffuseTexture : register(t0);
SamplerState DiffuseSampler : register(s0);

// Vertex Shader
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPos.xyz;
    output.Position = mul(worldPos, ViewProjection);
    output.Normal = normalize(mul(input.Normal, (float3x3)World));
    output.TexCoord = input.TexCoord;
    
    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET
{
    // Sample texture
    float4 texColor = DiffuseTexture.Sample(DiffuseSampler, input.TexCoord);
    float4 baseColor = texColor * MaterialColor;
    
    // Normalize vectors
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);
    float3 viewDir = normalize(CameraPosition - input.WorldPos);
    float3 halfDir = normalize(lightDir + viewDir);
    
    // Ambient
    float3 ambient = AmbientColor * baseColor.rgb;
    
    // Diffuse (Lambert)
    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = diff * LightColor * LightIntensity * baseColor.rgb;
    
    // Specular (Blinn-Phong)
    float spec = pow(max(dot(normal, halfDir), 0.0f), 32.0f);
    float3 specular = spec * LightColor * LightIntensity * 0.5f;
    
    // Combine
    float3 finalColor = ambient + diffuse + specular;
    
    return float4(finalColor, baseColor.a);
}
