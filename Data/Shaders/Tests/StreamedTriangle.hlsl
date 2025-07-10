//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:46:01
//

#include "Common/Bindless.hlsl"

#pragma vertex VSMain
#pragma fragment FSMain

static const float3 POSITIONS[] = {
    float3(-0.5f, -0.5f, 0.0f),
    float3( 0.5f, -0.5f, 0.0f),
    float3( 0.0f,  0.5f, 0.0f)
};

static const float3 COLORS[] = {
    float3(1.0f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.0f, 1.0f)
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VertexOutput VSMain(uint VertexID : SV_VertexID)
{
    VertexOutput output = (VertexOutput)0;

    output.Position = float4(POSITIONS[VertexID], 1.0f);
    output.Color = COLORS[VertexID];

    return output;  
}

float4 FSMain(VertexOutput input) : SV_Target
{
    return float4(input.Color, 1.0f);
}
