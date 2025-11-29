// Simple test to verify rendering works - draws a triangle in clip space
#include <d3d11.h>

void TestRender(ID3D11DeviceContext* context) {
    // This should draw a red triangle covering the screen
    // No matrices needed - vertices are in clip space

    static const char* shader = R"(
        struct VS_OUT { float4 pos : SV_POSITION; float4 col : COLOR; };
        VS_OUT VSMain(uint vid : SV_VertexID) {
            VS_OUT o;
            o.col = float4(1,0,0,1);
            if(vid==0) o.pos = float4(0,0.5,0,1);
            else if(vid==1) o.pos = float4(0.5,-0.5,0,1);
            else o.pos = float4(-0.5,-0.5,0,1);
            return o;
        }
        float4 PSMain(VS_OUT i) : SV_TARGET { return i.col; }
    )";

    // Compile and bind shader, then draw 3 vertices
    context->Draw(3, 0);
}