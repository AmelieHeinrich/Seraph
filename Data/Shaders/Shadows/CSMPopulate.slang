//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-09 18:38:45
//

#include "Common/Bindless.slang"

import Common.Shadow;
import Common.Camera;
import Common.Math;
import Common.Compute;
import Common.Lights;

struct Cascade
{
    uint SRVIndex;
    float Split;
    float2 Pad;

    column_major float4x4 View;
    column_major float4x4 Proj;
};

DEFINE_CBV_ARRAY(Camera);
DEFINE_SRV_ARRAY(DirectionalLight);
DEFINE_SRV_ARRAY(Cascade);

struct PushConstants
{
    uint CascadeArrayIndex;
    uint SamplerIndex;
    uint CameraIndex;
    uint DepthIndex;

    uint OutputIndex;
    uint SunIndex;
    uint NormalIndex;
    uint Pad;

    uint Width;
    uint Height;
    uint2 Pad1;
};
PUSH_CONSTANTS(PushConstants, Push);

float CalculateShadowCascade(float4 world, float3 N, float3 L, int layer)
{
    Camera camera = BindlessCBV_Camera.Load(Push.CameraIndex);
    StructuredBuffer<Cascade> cascades = BindlessSRV_Cascade.Load(Push.CascadeArrayIndex);

    float distanceToCamera = length(camera.Position.xyz - world.xyz);
    Cascade cascade0 = cascades[layer];
    Cascade cascade1 = cascades[min(layer + 1, SHADOW_CASCADE_COUNT - 1)];

    SamplerState sampler = BindlessSampler.Load(Push.SamplerIndex);
    Texture2D<float> shadowMap0 = BindlessTexture2DFloat.Load(cascade0.SRVIndex);
    Texture2D<float> shadowMap1 = BindlessTexture2DFloat.Load(cascade1.SRVIndex);

    float texelSize0 = cascade0.Split / SHADOW_MAP_SIZE;
    float texelSize1 = cascade1.Split / SHADOW_MAP_SIZE;

    float slopeBias = 0.05;
    float minBias = 0.005;

    float bias0 = max(slopeBias * (1.0 - dot(N, L)), minBias) * texelSize0;
    float bias1 = max(slopeBias * (1.0 - dot(N, L)), minBias) * texelSize1;

    int kernelSize = 1;

    float shadow0 = PCFCascade(shadowMap0, sampler, world, L, cascade0.View, cascade0.Proj, bias0, kernelSize);
    float shadow1 = PCFCascade(shadowMap1, sampler, world, L, cascade1.View, cascade1.Proj, bias1, kernelSize);

    // Compute blend factor near cascade split
    float blendRange = 0.1; // 10% of split range
    float blendStart = cascade0.Split * (1.0 - blendRange);
    float blendFactor = saturate((distanceToCamera - blendStart) / (cascade0.Split - blendStart));

    return lerp(shadow0, shadow1, blendFactor);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 dtid: SV_DispatchThreadID)
{
    // Start
    if (dtid.x >= Push.Width || dtid.y >= Push.Height)
        return;

    // Load resources
    RWTexture2D<float> output = BindlessRWTexture2DFloat.Load(Push.OutputIndex);
    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.DepthIndex);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.NormalIndex);
    StructuredBuffer<DirectionalLight> sun = BindlessSRV_DirectionalLight.Load(Push.SunIndex);
    StructuredBuffer<Cascade> cascades = BindlessSRV_Cascade.Load(Push.CascadeArrayIndex);
    Camera camera = BindlessCBV_Camera.Load(Push.CameraIndex);

    // Load data
    float viewDepth = depth.Load(dtid);
    float2 uv = TexelToUV(dtid.xy, float2(Push.Width, Push.Height));
    float3 cameraPosition = camera.Position.xyz;
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);
    float3 N = normalize(normal.Load(dtid).rgb);
    float3 L = -sun[0].Direction;

    int layer = -1;
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        if (abs(LinearizeDepth(viewDepth, CAMERA_NEAR, CAMERA_FAR)) < cascades[i].Split) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = SHADOW_CASCADE_COUNT - 1;
    }

    // Algorithm
    output[dtid.xy] = 1.0f * CalculateShadowCascade(worldPosition, N, L, layer);
}
