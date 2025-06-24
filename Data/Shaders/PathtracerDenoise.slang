//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 19:22:47
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint Now;
    uint Before;
    uint FrameCount;
    uint Pad;

    uint Width;
    uint Height;
    uint2 Pad2;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= Push.Width || tid.y >= Push.Height) return;

    RWTexture2D<float4> now = BindlessRWTexture2DFloat4.Load(Push.Now);
    Texture2D<float4> before = BindlessTexture2DFloat4.Load(Push.Before);
    now[tid.xy] = lerp(before.Load(tid), now[tid.xy], 1.0 / Push.FrameCount);
}
