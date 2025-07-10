//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:46:01
//

#include "Common/Bindless.hlsl"

#pragma vertex VSMain
#pragma fragment FSMain

struct VertexInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

VertexOutput VSMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;

    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;

    return output;  
}

float4 FSMain(VertexOutput input) : SV_Target
{
    return float4(input.Color, 1.0f);
}
