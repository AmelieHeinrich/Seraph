//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 18:12:10
//

#include "Common/Bindless.slang"

import Common.Camera;
import Common.Lights;

struct FrustumPlane
{
    float3 Normal;
    float Distance;
};

struct PushConstants
{
    uint LightIndex;
    uint CameraIndex;
    uint TileArray;
    uint BinsArray;

    uint TileWidth;
    uint TileHeight;
    uint NumTilesX;
    uint NumTilesY;

    uint Width;
    uint Height;
    uint PointLightCount;
    uint Pad;
};

DEFINE_UAV_ARRAY(uint);
DEFINE_UAV_ARRAY(TileData);
DEFINE_SRV_ARRAY(PointLight);
DEFINE_CBV_ARRAY(Camera);
PUSH_CONSTANTS(PushConstants, Push);

float2 PixelToNDC(int x, int y)
{
    float ndcX = (2.0f * x) / Push.Width - 1.0f;
    float ndcY = 1.0f - (2.0f * y) / Push.Height;
    return float2(ndcX, ndcY);
}

float4 NDCToView(float2 ndc, float z, column_major float4x4 invProj)
{
    float4 clip = float4(ndc, z, 1.0f);
    float4 view = mul(invProj, clip);
    return view / view.w;
}

FrustumPlane MakePlane(float3 a, float3 b, float3 c)
{
    FrustumPlane output = {};
    output.Normal = normalize(cross(b - a, c - a));
    output.Distance = -dot(output.Normal, a);
    return output;
}

bool SphereFrustumTest(float3 center, float radius, FrustumPlane plane)
{
    float distance = dot(plane.Normal, center) + plane.Distance;
    if (distance < -radius) return false;
    return true;
}

[shader("compute")]
[numthreads(32, 32, 1)]
void CSMain(uint3 tid: SV_DispatchThreadID)
{
    // Check
    if (tid.x >= Push.NumTilesX || tid.y >= Push.NumTilesY)
        return;

    // Get resources
    Camera camera = BindlessCBV_Camera.Load(Push.CameraIndex);
    StructuredBuffer<PointLight> pointLights = BindlessSRV_PointLight.Load(Push.PointLightCount);
    RWStructuredBuffer<TileData> tiles = BindlessUAV_TileData.Load(Push.TileArray);
    RWStructuredBuffer<uint> bins = BindlessUAV_uint.Load(Push.BinsArray);

    // Get tile index
    uint tx = tid.x;
    uint ty = tid.y;
    uint tileIndex = ty * Push.NumTilesX + tx;
    tiles[tileIndex].Offset = tileIndex * MAX_LIGHTS_PER_TILE;

    // Generate tile data
    int x0 = max(tx * Push.TileWidth, 0u);
    int y0 = max(ty * Push.TileHeight, 0u);
    int x1 = min(x0 + Push.TileWidth, Push.Width);
    int y1 = min(y0 + Push.TileHeight, Push.Height);

    float2 ndcTL = PixelToNDC(x0, y0);
    float2 ndcTR = PixelToNDC(x1, y0);
    float2 ndcBR = PixelToNDC(x1, y1);
    float2 ndcBL = PixelToNDC(x0, y1);

    float3 v0 = NDCToView(ndcTL, 0.0f, camera.InvProj).rgb;
    float3 v1 = NDCToView(ndcTR, 0.0f, camera.InvProj).rgb;
    float3 v2 = NDCToView(ndcBR, 0.0f, camera.InvProj).rgb;
    float3 v3 = NDCToView(ndcBL, 0.0f, camera.InvProj).rgb;
    float3 v4 = NDCToView(ndcTL, 1.0f, camera.InvProj).rgb;
    float3 v5 = NDCToView(ndcTR, 1.0f, camera.InvProj).rgb;
    float3 v6 = NDCToView(ndcBR, 1.0f, camera.InvProj).rgb;
    float3 v7 = NDCToView(ndcBL, 1.0f, camera.InvProj).rgb;

    FrustumPlane left   = MakePlane(v0, v3, v7);
    FrustumPlane right  = MakePlane(v2, v1, v5);
    FrustumPlane top    = MakePlane(v1, v0, v4);
    FrustumPlane bottom = MakePlane(v3, v2, v6);
    FrustumPlane near   = MakePlane(v0, v1, v2);
    FrustumPlane far    = MakePlane(v5, v4, v7);

    // Cull lights
    uint count = 0;
    for (uint i = 0; i < Push.PointLightCount; i++) {
        if (count >= MAX_LIGHTS_PER_TILE) break;

        PointLight light = pointLights[i];
        float3 viewPos = mul(camera.View, float4(light.Position, 1.0)).xyz;
        bool visible = SphereFrustumTest(viewPos, light.Radius, left)
                    && SphereFrustumTest(viewPos, light.Radius, right)
                    && SphereFrustumTest(viewPos, light.Radius, top)
                    && SphereFrustumTest(viewPos, light.Radius, bottom)
                    && SphereFrustumTest(viewPos, light.Radius, near)
                    && SphereFrustumTest(viewPos, light.Radius, far);
        if (visible) {
            bins[tiles[tileIndex].Offset + count] = i;
            count++;
        }
    }
    tiles[tileIndex].Count = count;
}
