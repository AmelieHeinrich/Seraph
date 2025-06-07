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
    uint Output;

    uint Width;
    uint Height;
    uint2 Pad;
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
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);

    output[tid.xy] = albedo.Load(tid);
}
