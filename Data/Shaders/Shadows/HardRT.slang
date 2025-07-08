//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:34:43
//

#include "Common/Bindless.slang"

import Common.Camera;
import Common.Lights;
import Common.Math;
import Common.Compute;

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

struct SceneInstance
{
    uint VertexBuffer;
    uint IndexBuffer;
    uint MaterialIndex;
    uint MaterialBuffer;
};

struct SceneMaterial
{
    uint Albedo;
    uint Normal;
    uint PBR;
    uint Pad;
};

DEFINE_SRV_ARRAY(SceneMaterial);
DEFINE_SRV_ARRAY(SceneInstance);
DEFINE_SRV_ARRAY(VertexInput);
DEFINE_SRV_ARRAY(uint);
DEFINE_SRV_ARRAY(DirectionalLight);
DEFINE_CBV_ARRAY(Camera);

struct PushConstants
{
    uint SunArray;
    uint Output;
    uint SceneAS;
    float NormalBias;

    uint Width;
    uint Height;
    uint GBufferDepth;
    uint GBufferNormal;

    uint CameraIndex;
    uint Sampler;
    uint SceneInstances;
    uint Pad;
};
PUSH_CONSTANTS(PushConstants, Push);

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
};

void CheckTransparency(inout RayQuery q)
{
    HitData data;
    data.LoadCandidate(q);
    if (data.AlbedoColor.a > 0.50) {
        q.CommitNonOpaqueTriangleHit();
    }
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dtid: SV_DispatchThreadID)
{
    if (dtid.x >= Push.Width || dtid.y >= Push.Height)
        return;

    // Load resources
    RWTexture2D<float> output = BindlessRWTexture2DFloat.Load(Push.Output);
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.GBufferDepth);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.GBufferNormal);
    RaytracingAccelerationStructure sceneAS = BindlessAccelerationStructure.Load(Push.SceneAS);
    StructuredBuffer<DirectionalLight> sun = BindlessSRV_DirectionalLight.Load(Push.SunArray);
    Camera camera = BindlessCBV_Camera.Load(Push.CameraIndex);

    // Load data
    float viewDepth = depth.Load(dtid);
    float2 uv = TexelToUV(dtid.xy, float2(Push.Width, Push.Height));
    float3 cameraPosition = camera.Position.xyz;
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);
    float3 N = normalize(normal.Load(dtid).rgb);
    float3 L = -sun[0].Direction;

    // Logic
    float attenuation = clamp(dot(N, L), 0.0, 1.0);
    if (attenuation > 0.0f) {
        // trace
        RayDesc ray;
        ray.Origin = worldPosition.xyz + N * Push.NormalBias;
        ray.Direction = L;
        ray.TMin = 0.001f;
        ray.TMax = 10000.0f;

        RayQuery<RAY_FLAG_NONE> q;
        q.TraceRayInline(sceneAS, RAY_FLAG_NONE, 0xFF, ray);
        while (q.Proceed()) {
            if (q.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE) {
                CheckTransparency(q);
            }
        }

        if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
            output[dtid.xy] = 0.0f;
        } else {
            output[dtid.xy] = 1.0f;
        }
    } else {
        output[dtid.xy] = 1.0f;
    }
}
