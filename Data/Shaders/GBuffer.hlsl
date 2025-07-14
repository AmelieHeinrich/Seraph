//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:05:29
//

#include "Common/Bindless.hlsl"
#include "Common/Camera.hlsl"

#pragma vertex VSMain
#pragma fragment FSMain

DEFINE_CBV_ARRAY(Camera);

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

struct VertexOutput
{
    float4 Position : SV_Position;
    float4 PrevPosition : POSITION0;
    float4 WorldPosition : POSITION1;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct FragmentOutput
{
    float4 Normal : SV_Target0;
    float4 Albedo : SV_Target1;
    float2 PBR : SV_Target2;
    float2 MotionVector : SV_Target3;
};

struct PushConstants
{
    uint MyTexture;
    uint MyNormalTexture;
    uint MyPBRTexture;
    uint MySampler; // 4 * uint = 16 bytes

    uint MyVertexBuffer;
    uint MyCameraBuffer;
    uint2 Pad;
};
PUSH_CONSTANTS(PushConstants, Push);

float3 GetNormal(VertexOutput input)
{
    if (Push.MyNormalTexture == INVALID_DESCRIPTOR)
        return normalize(input.Normal);

    Texture2D<float4> texture = BindlessTexture2DFloat4_Load(NonUniformResourceIndex(Push.MyNormalTexture));
    SamplerState sampler = BindlessSampler_Load(Push.MySampler);

    float3 normalSample = texture.Sample(sampler, input.UV).xyz * 2.0f - 1.0f;

    // Reconstruct TBN matrix
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent.xyz);
    float3 B = cross(N, T) * input.Tangent.w;

    float3x3 TBN = float3x3(T, B, N);

    // Transform normal to world space
    float3 worldNormal = normalize(mul(normalSample, TBN));
    return worldNormal;
}

float2 GetPBR(VertexOutput input)
{
    if (Push.MyPBRTexture == INVALID_DESCRIPTOR)
        return float2(0.0f, 0.0f);

    Texture2D<float4> texture = BindlessTexture2DFloat4_Load(NonUniformResourceIndex(Push.MyPBRTexture));
    SamplerState sampler = BindlessSampler_Load(Push.MySampler);

    float4 colorData = texture.Sample(sampler, input.UV);
    return float2(colorData.b, colorData.g);
}

VertexOutput VSMain(uint vid : SV_VertexID)
{
    Camera camera = BindlessCBV_Camera_Load(Push.MyCameraBuffer);
    StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput_Load(NonUniformResourceIndex(Push.MyVertexBuffer));
    VertexInput input = vertices[vid];

    VertexOutput output = (VertexOutput)0;
    output.Position = mul(camera.Proj, mul(camera.View, float4(input.Position, 1.0f)));
    output.PrevPosition = mul(camera.PrevProj, mul(camera.PrevView, float4(input.Position, 1.0f)));
    output.WorldPosition = float4(input.Position, 1.0f);
    output.Normal = input.Normal;
    output.UV = input.Texcoord;
    output.Tangent = input.Tangent;
    return output;
}

FragmentOutput FSMain(VertexOutput input)
{
    Texture2D<float4> texture = BindlessTexture2DFloat4_Load(NonUniformResourceIndex(Push.MyTexture));
    SamplerState sampler = BindlessSampler_Load(Push.MySampler);

    float2 ndcCurrent = input.Position.xy / input.Position.w;
    float2 ndcPrev = input.PrevPosition.xy / input.PrevPosition.w;
    
    float2 screenCurrent = ndcCurrent * 0.5 + 0.5;
    float2 screenPrev = ndcPrev * 0.5 + 0.5;
    screenCurrent.y = 1.0 - screenCurrent.y;
    screenPrev.y = 1.0 - screenPrev.y;

    FragmentOutput output = (FragmentOutput)0;
    output.Albedo = texture.Sample(sampler, input.UV);
    if (output.Albedo.a < 0.25)
        discard;
    output.Normal = float4(GetNormal(input), 1.0f);
    output.PBR = GetPBR(input);
    output.MotionVector = screenCurrent - screenPrev;
    return output;
}
