//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:46:01
//

#include "Common/Bindless.slang"

struct PushConstants
{
    uint OutputIndex;
};
PUSH_CONSTANTS(PushConstants, Push);

[shader("compute")]
[numthreads(8, 8, 1)]
void CSMain(uint3 tid : SV_DispatchThreadID)
{
    RWTexture2D<float4> output = BindlessRWTexture2DFloat4.Load(Push.OutputIndex);

    uint width, height;
    output.GetDimensions(width, height);

    if (tid.x < width && tid.y < height)
    {
        output[tid.xy] = float4(float(tid.x) / width, float(tid.y) / height, 0.0f, 1.0f);
    }
}
