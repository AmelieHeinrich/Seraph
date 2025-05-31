//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 16:41:37
//

import Bindless;

struct VertexInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
};

struct VertexOutput
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD;
};

struct PushConstants
{
    DescriptorHandle<Texture2D<float4>> MyTexture;
    DescriptorHandle<SamplerState> MySampler;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> Push : register(b0);

[shader("vertex")]
VertexOutput VSMain(VertexInput input)
{
    VertexOutput output = {};

    output.Position = float4(input.Position, 1.0f);
    output.UV = input.UV;

    return output;
}

[shader("fragment")]
float4 FSMain(VertexOutput input) : SV_Target
{
    Texture2D<float4> texture = getDescriptorFromHandle(Push.MyTexture);
    SamplerState sampler = getDescriptorFromHandle(Push.MySampler);

    return texture.Sample(sampler, input.UV);
}
