//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 16:41:37
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

struct CBVStruct
{
    float4 TintColor;
};
DEFINE_CBV_ARRAY(CBVStruct);

struct PushConstants
{
    uint MyTexture;
    uint MySampler;
    uint MyCBV;
};
PUSH_CONSTANTS(PushConstants, Push);

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
    Texture2D<float4> texture = BindlessTexture2D<float4>.Load(Push.MyTexture);
    SamplerState sampler = BindlessSampler.Load(Push.MySampler);
    CBVStruct cbv = BindlessCBV_CBVStruct.Load(Push.MyCBV);

    return texture.Sample(sampler, input.UV) * cbv.TintColor;
}
