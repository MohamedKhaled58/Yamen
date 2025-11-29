cbuffer CBPerObject : register(b0)
{
    float4x4 u_MVP;
};

struct VS_Input
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct PS_Input
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PS_Input VSMain(VS_Input input)
{
    PS_Input output;
    output.Position = mul(float4(input.Position, 1.0f), u_MVP);
    output.Color = input.Color;
    return output;
}

float4 PSMain(PS_Input input) : SV_TARGET
{
    return input.Color;
}
