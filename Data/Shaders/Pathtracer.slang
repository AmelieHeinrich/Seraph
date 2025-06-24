//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:59:59
//

#include "Common/Bindless.slang"

import Common.Camera;
import Common.Compute;
import Common.Math;

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
DEFINE_SRV_ARRAY(uint);

struct SceneInstance
{
    uint VertexBuffer;
    uint IndexBuffer;
    uint MaterialIndex;
    uint MaterialBuffer;
};
DEFINE_SRV_ARRAY(SceneInstance);

struct SceneMaterial
{
    uint Albedo;
    uint Normal;
    uint PBR;
    uint Pad;
};
DEFINE_SRV_ARRAY(SceneMaterial);

struct PushConstants
{
    uint Width;
    uint Height;
    uint Output;
    uint Albedo;

    uint Camera;
    uint Depth;
    uint AS;
    uint Normal;

    uint SceneInstances;
    uint Sampler;
    uint2 Pad;
};
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    // Load resources
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.Albedo);
    Texture2D<float4> normalTexture = BindlessTexture2DFloat4.Load(Push.Normal);
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.Depth);
    RaytracingAccelerationStructure rtAS = BindlessAccelerationStructure.Load(Push.AS);
    StructuredBuffer<SceneInstance> instances = BindlessSRV_SceneInstance.Load(Push.SceneInstances);
    SamplerState sampler = BindlessSampler.Load(Push.Sampler);
    Camera camera = BindlessCBV_Camera.Load(Push.Camera);

    // Work
    float4 normal = normalTexture.Load(tid);
    float viewDepth = depth.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);

    // Setup initial ray
    RayDesc ray;
    ray.Origin = worldPosition.xyz + normal.rgb * 0.001;
    ray.Direction = float3(0.0, 1.0, 0.0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayQuery<RAY_FLAG_NONE> q;
    q.TraceRayInline(rtAS, RAY_FLAG_NONE, 0xFF, ray);

    // Check for transparency
    while (q.Proceed())
    {
        if (q.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE) {
            uint instanceIndex = q.CandidateInstanceIndex();
            uint primIndex = q.CandidatePrimitiveIndex();
            float2 bary = q.CandidateTriangleBarycentrics();

            SceneInstance instance = instances[instanceIndex];
            StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(instance.VertexBuffer);
            StructuredBuffer<uint> indices = BindlessSRV_uint.Load(instance.IndexBuffer);
            StructuredBuffer<SceneMaterial> materials = BindlessSRV_SceneMaterial.Load(instance.MaterialBuffer);

            SceneMaterial material = materials[instance.MaterialIndex];
            Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(material.Albedo);

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
            if (color.a > 0.25) {
                q.CommitNonOpaqueTriangleHit();
            }
        }
    }

    // Main work
    float shadowFactor = 1.0f;
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        shadowFactor = 0.0f;
    } else {
        shadowFactor = 1.0f;
    }

    // Output
    output[tid.xy] = float4(albedo.Load(tid).rgb * shadowFactor, 1.0f);
}
