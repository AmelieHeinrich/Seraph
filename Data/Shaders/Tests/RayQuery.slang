//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 11:52:33
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint Output;
    uint AccelerationStructure;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    RWTexture2D<float4> output = BindlessRWTexture2D<float4>.Load(Push.Output);
    RaytracingAccelerationStructure accelerationStructure = BindlessAccelerationStructure.Load(Push.AccelerationStructure);

    uint width, height;
    output.GetDimensions(width, height);
    if (tid.x >= width || tid.y >= height) return;

    float2 lerpValues = (float2)tid.xy / float2(width, height);
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

    RayQuery q;
    q.TraceRayInline(accelerationStructure, RAY_FLAG_NONE, 0xFF, ray);
    q.Proceed();

    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        float2 barycentrics = q.CommittedTriangleBarycentrics();
        color = float4(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y, 1.0f);
    }
    output[tid.xy] = color;
}
