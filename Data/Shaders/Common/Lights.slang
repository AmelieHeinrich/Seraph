//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 20:18:51
//

#include "Bindless.slang"

module Lights;

static const uint MAX_POINT_LIGHTS = 16384;

struct PointLight
{
    float3 Position;
    float Radius;

    float3 Color;
    uint ShadowMapSRV;
};
DEFINE_SRV_ARRAY(PointLight);
