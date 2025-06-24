//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 08:21:27
//

#include "Common/Bindless.slang"

struct VertexInput
{
    float3 Position;
    float _pad0;

    float2 Texcoord;
    float2 _pad1;
};
DEFINE_SRV_ARRAY(VertexInput);
DEFINE_SRV_ARRAY(uint);

struct PushConstants
{
    uint Output;
    uint AccelerationStructure;
    uint Width;
    uint Height;

    uint VBV;
    uint IBV;
    uint SRV;
    uint Sampler;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    RaytracingAccelerationStructure accelerationStructure = BindlessAccelerationStructure.Load(Push.AccelerationStructure);
    StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(Push.VBV);
    StructuredBuffer<uint> indices = BindlessSRV_uint.Load(Push.IBV);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.SRV);
    SamplerState sampler = BindlessSampler.Load(Push.Sampler);

    float2 lerpValues = (float2)tid.xy / float2(Push.Width, Push.Height);
    float3 rayDir = float3(0, 0, 1);
    float3 rayOrigin = float3(
        lerp(-1.0f, 1.0f, lerpValues.x),
        lerp(-1.0f, 1.0f, lerpValues.y),
        0.0f
    );

    RayDesc ray;
    ray.Origin = rayOrigin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(accelerationStructure, RAY_FLAG_NONE, 0xFF, ray);
    while (q.Proceed())
    {
        if (q.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE) {
            float2 bary = q.CandidateTriangleBarycentrics();
            uint primIndex = q.CandidatePrimitiveIndex();

            uint3 triangle = uint3(
                indices[primIndex * 3 + 0],
                indices[primIndex * 3 + 1],
                indices[primIndex * 3 + 2]
            );

            VertexInput v0 = vertices[triangle.x];
            VertexInput v1 = vertices[triangle.y];
            VertexInput v2 = vertices[triangle.z];

            float3 interpolate = float3(
                1.0 - bary.x - bary.y,
                bary.x,
                bary.y
            );
            float2 uv = v0.Texcoord * interpolate.x + v1.Texcoord * interpolate.y + v2.Texcoord * interpolate.z;
            float4 color = albedo.SampleLevel(sampler, uv, 0.0);
            if (color.a > 0.25)
                q.CommitNonOpaqueTriangleHit();
        }
    }

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        float2 bary = q.CommittedTriangleBarycentrics();
        uint primIndex = q.CommittedPrimitiveIndex();

        uint3 triangle = uint3(
            indices[primIndex * 3 + 0],
            indices[primIndex * 3 + 1],
            indices[primIndex * 3 + 2]
        );

        VertexInput v0 = vertices[triangle.x];
        VertexInput v1 = vertices[triangle.y];
        VertexInput v2 = vertices[triangle.z];

        float3 interpolate = float3(
            1.0 - bary.x - bary.y,
            bary.x,
            bary.y
        );
        float2 uv = v0.Texcoord * interpolate.x + v1.Texcoord * interpolate.y + v2.Texcoord * interpolate.z;

        color = float4(albedo.SampleLevel(sampler, uv, 0.0).rgb, 1.0f);
    }
    output[tid.xy] = color;
}
