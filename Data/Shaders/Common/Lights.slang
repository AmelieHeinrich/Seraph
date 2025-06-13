//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 20:18:51
//

#include "Bindless.slang"

module Lights;

static const uint MAX_POINT_LIGHTS = 16384;

public struct PointLight
{
    public float3 Position;
    public float Radius;

    public float3 Color;
    public uint ShadowMapSRV;
};
