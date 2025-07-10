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

VertexOutput VSMain(VertexInput input)
{
    VertexOutput output = (VertexOutput)0;

    output.Position = float4(input.Position, 1.0f);
    output.UV = input.UV;

    return output;  
}

float4 FSMain(VertexOutput input) : SV_Target
{
    Texture2D<float4> srv = BindlessTexture2DFloat4_Load(Push.SRV);
    SamplerState sampler = BindlessSampler_Load(Push.Sampler);

    return srv.Sample(sampler, input.UV);
}
