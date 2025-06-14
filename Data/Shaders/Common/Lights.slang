//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 20:18:51
//

#include "Bindless.slang"

module Lights;

static public const uint MAX_POINT_LIGHTS = 16384;
static public const uint MAX_LIGHTS_PER_TILE = 1024;

public struct PointLight
{
    public float3 Position;
    public float Radius;

    public float3 Color;
    public float Intensity;
};

public struct TileData
{
    public uint Offset;
    public uint Count;
    public uint2 Pad;
};
