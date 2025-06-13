//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:40:28
//

module Compute;

public float2 TexelToUV(uint2 texel, float2 dispatchSize)
{
    return ((float2)texel + 0.5f) / dispatchSize;
}
