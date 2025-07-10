//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 20:18:51
//

static const uint MAX_POINT_LIGHTS = 16384;
static const uint MAX_SPOT_LIGHTS = 16384;
static const uint MAX_LIGHTS_PER_TILE = 1024;

struct PointLight
{
    float3 Position;
    float Radius;

    float3 Color;
    float Intensity;
};

struct SpotLight
{
    float3 Position;
    float Size;

    float3 Forward;
    float Angle;

    float3 Color;
    float Intensity;
};

struct DirectionalLight
{
    float3 Direction;
    float Intensity;

    float3 Color;
    float Pad;
};

struct TileData
{
    uint Offset;
    uint Count;
    float MinDepth;
    float MaxDepth;
};
