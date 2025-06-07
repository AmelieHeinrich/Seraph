//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:05:29
//

#include "Common/Bindless.slang"

struct VertexInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VertexOutput
{
    float4 Position : SV_Position;
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
    uint MySampler;
    uint2 Pad;

    column_major float4x4 View;
    column_major float4x4 Projection;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("vertex")]
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output = {};

    output.Position = mul(Push.Projection, mul(Push.View, float4(input.Position, 1.0f)));
    output.Normal = input.Normal;
    output.UV = input.UV;

    return output;
}

[shader("fragment")]
FragmentOutput FSMain(VertexOutput input)
{
    Texture2D<float4> texture = BindlessTexture2DFloat4.Load(Push.MyTexture);
    SamplerState sampler = BindlessSampler.Load(Push.MySampler);

    FragmentOutput output = {};
    output.Normal = float4(input.Normal, 1.0);
    output.Albedo = texture.Sample(sampler, input.UV);
    if (output.Albedo.a < 0.25)
        discard;
    return output;
}
