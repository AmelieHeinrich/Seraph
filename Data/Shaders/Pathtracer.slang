//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:59:59
//

#include "Common/Bindless.slang"

static const uint SAMPLE_COUNT = 1;
static const uint BOUNCE_COUNT = 2;

import Common.Camera;
import Common.Compute;
import Common.Math;
import Common.Random;
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
    uint FrameCount;
    uint BounceCount;
};
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

float3 GetNormal(VertexInput input, uint normalIndex, SamplerState sampler)
{
    if (normalIndex == INVALID_DESCRIPTOR)
        return normalize(input.Normal);

    Texture2D<float4> texture = BindlessTexture2DFloat4.Load(NonUniformResourceIndex(normalIndex));
    float3 normalSample = texture.SampleLevel(sampler, input.Texcoord, 0).xyz * 2.0f - 1.0f;

    // Reconstruct TBN matrix
    float3 N = normalize(input.Normal);
    float3 T = normalize(input.Tangent.xyz);
    float3 B = cross(N, T) * input.Tangent.w;

    float3x3 TBN = float3x3(T, B, N);

    // Transform normal to world space
    float3 worldNormal = normalize(mul(normalSample, TBN));
    return worldNormal;
}

struct HitData
{
    float3 HitPos;
    VertexInput InputVertex;
    float4 AlbedoColor;

    [mutating]
    void LoadCandidate(in RayQuery q)
    {
        uint instanceIndex = q.CandidateInstanceIndex();
        uint primIndex = q.CandidatePrimitiveIndex();
        float2 bary = q.CandidateTriangleBarycentrics();

        StructuredBuffer<SceneInstance> instances = BindlessSRV_SceneInstance.Load(Push.SceneInstances);
        SceneInstance instance = instances[instanceIndex];
        StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(instance.VertexBuffer);
        StructuredBuffer<uint> indices = BindlessSRV_uint.Load(instance.IndexBuffer);
        StructuredBuffer<SceneMaterial> materials = BindlessSRV_SceneMaterial.Load(instance.MaterialBuffer);

        SceneMaterial material = materials[instance.MaterialIndex];
        Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(material.Albedo);
        SamplerState sampler = BindlessSampler.Load(Push.Sampler);

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

        InputVertex.Position = HitPos;
        InputVertex.Texcoord = v0.Texcoord * interpolate.x + v1.Texcoord * interpolate.y + v2.Texcoord * interpolate.z;
        InputVertex.Normal = normalize(
            (1.0 - interpolate.x - interpolate.y) * v0.Normal +
            interpolate.x * v1.Normal +
            interpolate.y * v2.Normal
        );
        InputVertex.Tangent = normalize(
            (1.0 - interpolate.x - interpolate.y) * v0.Tangent +
            interpolate.x * v1.Tangent +
            interpolate.y * v2.Tangent
        );

        AlbedoColor = albedo.SampleLevel(sampler, InputVertex.Texcoord, 0);
    }

    [mutating]
    void Load(in RayQuery q)
    {
        HitPos = q.WorldRayOrigin() + q.CommittedRayT() * q.WorldRayDirection();

        uint instanceIndex = q.CommittedInstanceIndex();
        uint primIndex = q.CommittedPrimitiveIndex();
        float2 bary = q.CommittedTriangleBarycentrics();

        StructuredBuffer<SceneInstance> instances = BindlessSRV_SceneInstance.Load(Push.SceneInstances);
        SceneInstance instance = instances[instanceIndex];
        StructuredBuffer<VertexInput> vertices = BindlessSRV_VertexInput.Load(instance.VertexBuffer);
        StructuredBuffer<uint> indices = BindlessSRV_uint.Load(instance.IndexBuffer);
        StructuredBuffer<SceneMaterial> materials = BindlessSRV_SceneMaterial.Load(instance.MaterialBuffer);

        SceneMaterial material = materials[instance.MaterialIndex];
        Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(material.Albedo);
        SamplerState sampler = BindlessSampler.Load(Push.Sampler);

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

        InputVertex.Position = HitPos;
        InputVertex.Texcoord = v0.Texcoord * interpolate.x + v1.Texcoord * interpolate.y + v2.Texcoord * interpolate.z;
        InputVertex.Normal = normalize(
            (1.0 - interpolate.x - interpolate.y) * v0.Normal +
            interpolate.x * v1.Normal +
            interpolate.y * v2.Normal
        );
        InputVertex.Tangent = normalize(
            (1.0 - interpolate.x - interpolate.y) * v0.Tangent +
            interpolate.x * v1.Tangent +
            interpolate.y * v2.Tangent
        );
        InputVertex.Normal = GetNormal(InputVertex, material.Normal, sampler);

        AlbedoColor = albedo.SampleLevel(sampler, InputVertex.Texcoord, 0);
    }
};

void CheckTransparency(inout RayQuery q)
{
    HitData data;
    data.LoadCandidate(q);
    if (data.AlbedoColor.a > 0.50) {
        q.CommitNonOpaqueTriangleHit();
    }
}

RayDesc GeneratePrimaryRay(inout RNG rng, uint2 tid)
{
    Camera camera = BindlessCBV_Camera.Load(Push.Camera);

    float2 offset = float2(rng.NextFloat() - 0.5, rng.NextFloat() - 0.5);
    float3 vOrigin = 0.0;
    float3 vDirection = 0.0;

    const float2 pixelCenter = (tid.xy + 0.5) + offset;
    const float2 inUV = pixelCenter / float2(Push.Width, Push.Height);
    float2 d = (inUV * 2.0 - 1.0);

    vOrigin = mul(camera.InvView, float4(0, 0, 0, 1)).xyz;
    float4 target = mul(camera.InvProj, float4(d.x, -d.y, 1, 1));
    vDirection = mul(camera.InvView, float4(normalize(target.xyz), 0)).xyz;

    RayDesc ray;
    ray.TMin = 0.001;
    ray.TMax = 1000.0;
    ray.Origin = vOrigin;
    ray.Direction = vDirection;
    return ray;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    // Load resources
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    Texture2D<float4> albedoTex = BindlessTexture2DFloat4.Load(Push.Albedo);
    Texture2D<float4> normalTex = BindlessTexture2DFloat4.Load(Push.Normal);
    Texture2D<float> depthTex = BindlessTexture2DFloat.Load(Push.Depth);

    RaytracingAccelerationStructure rtAS = BindlessAccelerationStructure.Load(Push.AS);
    StructuredBuffer<SceneInstance> instances = BindlessSRV_SceneInstance.Load(Push.SceneInstances);
    SamplerState sampler = BindlessSampler.Load(Push.Sampler);
    Camera camera = BindlessCBV_Camera.Load(Push.Camera);

    // Read depth and early out if it's 1 (assumed to mean no geometry)
    float viewDepth = depthTex.Load(tid);
    if (viewDepth >= 1.0)
    {
        float3 skyColor = 1;
        output[tid.xy] = float4(skyColor, 1.0);
        return;
    }

    // Gather G-buffer info
    float4 normalData = normalTex.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float4 worldPosition4 = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);

    float3 worldPosition = worldPosition4.xyz;
    float3 normal = normalize(normalData.xyz);
    float3 baseColor = albedoTex.Load(tid).rgb;

    float3 finalColor = 0;
    float3 throughput = baseColor;
    bool alive = true;

    RNG rng;
    rng.Init(tid.xy, Push.FrameCount * 7919 * 104729);

    // Start at surface point
    RayDesc ray;
    ray.Origin = worldPosition + normal * 0.001;
    ray.Direction = rng.NextHemisphereUnitVector(normal);
    ray.TMin = 0.001;
    ray.TMax = 1000;

    for (int bounce = 0; bounce < Push.BounceCount; bounce++) {
        RayQuery<RAY_FLAG_NONE> q;
        q.TraceRayInline(rtAS, RAY_FLAG_NONE, 0xFF, ray);

        while (q.Proceed()) {
            if (q.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE) {
                CheckTransparency(q);
            }
        }

        if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
            HitData data;
            data.Load(q);

            float3 newNormal = data.InputVertex.Normal;
            float3 newOrigin = data.HitPos + newNormal * 0.001;
            float3 newDirection = rng.NextHemisphereUnitVector(newNormal);

            float cosTheta = dot(newNormal, newDirection);
            float pdf = cosTheta / PI;
            float3 f_r = data.AlbedoColor.rgb / PI;

            throughput *= f_r * cosTheta / pdf;
            ray.Origin = newOrigin;
            ray.Direction = newDirection;
        } else {
            float3 skyColor = 1;
            finalColor += throughput * skyColor;
            alive = false;
            break;
        }
    }

    output[tid.xy] = float4(finalColor, 1.0);
}
