//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:46:01
//

#include "Common/Bindless.slang"

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

[shader("vertex")]
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output = {};

    output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;

    return output;  
}

[shader("fragment")]
float4 FSMain(VertexOutput input) : SV_Target
{
    return float4(input.Color, 1.0f);
}
