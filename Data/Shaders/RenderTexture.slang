//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 17:28:54
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint In;
    uint Sampler;
};
PUSH_CONSTANTS(PushConstants, Push);

struct VertexOutput
{
    float4 Pos : SV_Position;
    float2 Texcoord : TEXCOORD;
};

[shader("vertex")]
VertexOutput VSMain(uint vid : SV_VertexID)
{
    VertexOutput output;

    // Fullscreen triangle vertices
    float2 positions[3] = {
        float2(-1.0f, -1.0f),
        float2(3.0f, -1.0f),
        float2(-1.0f, 3.0f)
    };

    float2 uvs[3] = {
        float2(0.0f, 1.0f),
        float2(2.0f, 1.0f),
        float2(0.0f, -1.0f)
    };

    output.Pos = float4(positions[vid], 0.0f, 1.0f);
    output.Texcoord = uvs[vid];
    return output;
}

[shader("fragment")]
float4 FSMain(VertexOutput output) : SV_Target
{
    Texture2D<float4> input = BindlessTexture2DFloat4.Load(Push.In);
    SamplerState state = BindlessSampler.Load(Push.Sampler);

    return input.Sample(state, output.Texcoord);
}
