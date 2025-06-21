//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:59:59
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint Width;
    uint Height;
    uint Output;
    uint Albedo;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.Output);
    Texture2D<float4> albedo = BindlessTexture2DFloat4.Load(Push.Albedo);

    output[tid.xy] = albedo.Load(tid);
}
