//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:59:59
//

#include "Common/Bindless.slang"

import Common.Camera;
import Common.Compute;
import Common.Math;

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
    Camera camera = BindlessCBV_Camera.Load(Push.Camera);

    // Work
    float4 normal = normalTexture.Load(tid);
    float viewDepth = depth.Load(tid);
    float2 uv = TexelToUV(tid.xy, float2(Push.Width, Push.Height));
    float4 worldPosition = GetWorldFromDepth(uv, viewDepth, camera.InvViewProj);

    RayDesc ray;
    ray.Origin = worldPosition.xyz + normal.rgb * 0.001;
    ray.Direction = float3(0.0, 1.0, 0.0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    RayQuery<RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(rtAS, RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.Proceed();

    float shadowFactor = 1.0f;
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        shadowFactor = 0.1f;
    } else {
        shadowFactor = 1.0f;
    }

    // Output
    output[tid.xy] = float4(albedo.Load(tid).rgb * shadowFactor, 1.0f);
}
