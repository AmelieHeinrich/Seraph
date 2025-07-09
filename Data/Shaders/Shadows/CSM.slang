//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-09 18:04:46
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

    float4 Tangent;
};
DEFINE_SRV_ARRAY(VertexInput);

struct PushConstants
{
    uint MyVertexBuffer;
    uint Albedo;
    uint Sampler;
    uint Pad;

    column_major float4x4 LightView;
    column_major float4x4 LightProj;
};
PUSH_CONSTANTS(PushConstants, Push);

struct VertexOut
{
    float4 Clip : SV_Position;
    float2 UV : TEXCOORD;
};

[shader("vertex")]
VertexOut VSMain(uint vid : SV_VertexID)
{
    // Load resources
    StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(NonUniformResourceIndex(Push.MyVertexBuffer));
    
    // Logic
    VertexInput input = vertices[vid];
    VertexOut output = {};
    output.Clip = mul(Push.LightProj, mul(Push.LightView, float4(input.Position, 1.0f)));
    output.UV = input.Texcoord;
    return output;
}

[shader("fragment")]
void PSMain(VertexOut input)
{
    Texture2D<float4> texture = BindlessTexture2DFloat4.Load(NonUniformResourceIndex(Push.Albedo));
    SamplerState sampler = BindlessSampler.Load(Push.Sampler);

    float4 color = texture.Sample(sampler, input.UV);
    if (color.a > 0.50)
        discard;
}
