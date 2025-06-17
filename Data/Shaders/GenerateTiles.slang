//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-17 13:50:06
//

#include "Common/Bindless.slang"

import Common.Lights;

struct PushConstants
{
    uint TileArray;
    uint TileWidth;
    uint TileHeight;
    uint DepthMap;

    uint NumTilesX;
    uint NumTilesY;
    uint Width;
    uint Height;
};
DEFINE_UAV_ARRAY(TileData);
PUSH_CONSTANTS(PushConstants, Push);

groupshared uint gMinDepth;
groupshared uint gMaxDepth;

[shader("compute")]
[numthreads(16, 16, 1)]
void CSMain(uint3 gid: SV_GroupID, uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID)
{
    if (gid.x >= Push.NumTilesX || gid.y >= Push.NumTilesY)
        return;

    RWStructuredBuffer<TileData> tiles = BindlessUAV_TileData.Load(Push.TileArray);
    Texture2D<float> depthMap = BindlessTexture2DFloat.Load(Push.DepthMap);
    uint tileIndex = gid.y * Push.NumTilesX + gid.x;

    // Step 1. Initialize default values
    if (gtid.x == 0 && gtid.y == 0) {
        gMinDepth = 0xFFFFFFFF;
        gMaxDepth = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    // Step 2. Load the depth buffer and see whats the min/max value. Store in NDC because we generate the frustum in view space in the cull shader
    uint2 pixelCoord = uint2(
        gid.x * Push.TileWidth + gtid.x,
        gid.y * Push.TileHeight + gtid.y,
    );

    if (pixelCoord.x >= Push.Width || pixelCoord.y >= Push.Height)
        return;
    float depthValue = depthMap.Load(uint3(pixelCoord, 0));
    if (depthValue > 0.0f && depthValue < 1.0f) {
        uint depthInt = asuint(depthValue);
        InterlockedMin(gMinDepth, depthInt);
        InterlockedMax(gMaxDepth, depthInt);
    }

    GroupMemoryBarrierWithGroupSync();

    // Step 3. Write result
    if (gMinDepth == 0xFFFFFFFF && gMaxDepth == 0) {
        gMinDepth = asuint(0.0f);
        gMaxDepth = asuint(1.0f);
    }

    tiles[tileIndex].MinDepth = asfloat(gMinDepth);
    tiles[tileIndex].MaxDepth = asfloat(gMaxDepth);
}
