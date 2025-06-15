//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:55:27
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* LIGHT_CULL_TILE_BUFFER = "LightCulling/TileBuffer";
constexpr const char* LIGHT_CULL_TILE_INDICES_BUFFER = "LightCulling/TileIndicesBuffer";
constexpr uint MAX_LIGHT_PER_TILE = 1024;
constexpr uint TILE_WIDTH = 16;
constexpr uint TILE_HEIGHT = 16;

struct LightTile
{
    uint Offset;
    uint Count;
    uint2 Pad;
};

class LightCulling : public RenderPass
{
public:
    LightCulling(IRHIDevice* device, uint width, uint height);
    ~LightCulling();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    IRHIComputePipeline* mCullPipeline;

private:
    uint mNumTilesX;
    uint mNumTilesY;
};
