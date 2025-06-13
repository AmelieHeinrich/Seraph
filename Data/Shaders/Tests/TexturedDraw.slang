//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:46:01
//

#include "Common/Bindless.slang"

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

struct PushConstant
{
    uint SRV;
    uint Sampler;
    uint2 Pad;
};

PUSH_CONSTANTS(PushConstant, Push);

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
    Texture2D<float4> srv = BindlessTexture2DFloat4.Load(Push.SRV);
    SamplerState sampler = BindlessSampler.Load(Push.Sampler);

    return srv.Sample(sampler, input.UV);
}
