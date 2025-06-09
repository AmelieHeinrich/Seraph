//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-09 16:31:31
//

#include "Common/Bindless.slang"

struct VertexIn
{
    float3 Position : POSITION;
    float Pad;

    float3 Color : COLOR;
    float Pad1;
};
DEFINE_SRV_ARRAY(VertexIn);

struct VertexOut
{
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

struct Settings
{
    column_major float4x4 Projection;
    column_major float4x4 View;

    uint MyVertexBuffer;
    uint3 Pad;
};
PUSH_CONSTANTS(Settings, PushConstants);

[shader("vertex")]
VertexOut VSMain(uint vid : SV_VertexID)
{
    StructuredBuffer<VertexIn> vertices = BindlessSRV_VertexIn.Load(PushConstants.MyVertexBuffer);
    VertexIn Input = vertices[vid];

    VertexOut Output = (VertexOut)0;
    Output.Position = float4(Input.Position, 1.0);
    Output.Position = mul(PushConstants.View, Output.Position);
    Output.Position = mul(PushConstants.Projection, Output.Position);
    Output.Color = Input.Color;
    return Output;
}

[shader("fragment")]
float4 FSMain(VertexOut Input) : SV_Target
{
    return float4(Input.Color, 1.0);
}
