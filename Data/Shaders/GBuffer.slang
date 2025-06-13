//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:05:29
//

#include "Common/Bindless.slang"

struct VertexInput
{
    float3 Position;
    float _pad0;

    float3 Normal;
    float _pad1;

    float2 Texcoord;
    float2 _pad2;

    float3 Tangent;
    float _pad3;

    float3 Bitangent;
    float _pad4;
};
DEFINE_SRV_ARRAY(VertexInput);

struct VertexOutput
{
    float4 Position : SV_Position;
    float4 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct FragmentOutput
{
    float4 Normal : SV_Target0;
    float4 Albedo : SV_Target1;
};

struct PushConstants
{
    uint MyTexture;
    uint MyNormalTexture;
    uint MySampler;
    uint MyVertexBuffer;

    column_major float4x4 View;
    column_major float4x4 Projection;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("vertex")]
VertexOutput VSMain(uint vid : SV_VertexID)
{
    StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(Push.MyVertexBuffer);
    VertexInput input = vertices[vid];

    VertexOutput output = {};
    output.Position = mul(Push.Projection, mul(Push.View, float4(input.Position, 1.0f)));
    output.WorldPosition = float4(input.Position, 1.0f);
    output.Normal = input.Normal;
    output.UV = input.Texcoord;
    return output;
}

[shader("fragment")]
FragmentOutput FSMain(VertexOutput input)
{
    Texture2D<float4> texture = BindlessTexture2DFloat4.Load(Push.MyTexture);
    SamplerState sampler = BindlessSampler.Load(Push.MySampler);

    FragmentOutput output = {};
    output.Normal = float4(input.Normal, 1.0f);
    output.Albedo = output.Normal;
    return output;
}
