//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 15:18:07
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint Depth;
    uint Normal;
    uint Albedo;
    uint PBR;
    uint Output;

    uint Width;
    uint Height;
    uint Pad;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    Texture2D<float> depth = BindlessTexture2DFloat.Load(Push.Depth);
    Texture2D<float4> normal = BindlessTexture2DFloat4.Load(Push.Normal);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.Albedo);
    Texture2D<float2> pbr = BindlessTexture2DFloat2.Load(Push.PBR);
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);

    float metallic = pbr.Load(tid).r;
    float roughness = pbr.Load(tid).g;

    output[tid.xy] = roughness;
}
