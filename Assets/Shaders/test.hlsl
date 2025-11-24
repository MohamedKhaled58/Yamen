// Simple test shader - clear color
// Vertex Shader
struct VSInput {
    float3 position : POSITION;
};

struct VSOutput {
    float4 position : SV_POSITION;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = float4(input.position, 1.0f);
    return output;
}

// Pixel Shader
float4 PSMain(VSOutput input) : SV_TARGET {
    return float4(0.2f, 0.4f, 0.8f, 1.0f); // Cornflower blue
}
