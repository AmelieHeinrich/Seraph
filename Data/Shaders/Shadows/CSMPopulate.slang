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

DEFINE_CBV_ARRAY(Camera);
DEFINE_SRV_ARRAY(DirectionalLight);

struct Cascade
{
    uint SRVIndex;
    float Split;
    float2 Pad;

    column_major float4x4 View;
    column_major float4x4 Proj;
};
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

    Cascade cascade = cascades[layer];
    SamplerState sampler = BindlessSampler.Load(Push.SamplerIndex);
    Texture2D<float> shadowMap = BindlessTexture2DFloat.Load(cascade.SRVIndex);

    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
    if (layer == SHADOW_CASCADE_COUNT) {
        bias *= 1 / (CAMERA_FAR * 0.5);
    } else {
        bias *= 1 / (cascade.Split * 0.5);
    }

    int kernelSize = 1;
    return PCFCascade(shadowMap,
                      sampler,
                      world,
                      L,
                      cascade.View,
                      cascade.Proj,
                      bias,
                      kernelSize);
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
