//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:41:55
//

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* LIGHT_CULL_TILE_BUFFER = "LightCulling/TileBuffer";
    constexpr const char* LIGHT_CULL_TILE_INDICES_BUFFER = "LightCulling/TileIndicesBuffer";
    
    constexpr uint MAX_LIGHT_PER_TILE = 1024;
    constexpr uint TILE_WIDTH = 16;
    constexpr uint TILE_HEIGHT = 16;

    struct LightTile
    {
        uint Offset;
        uint Count;
        float MinDepth;
        float MaxDepth;
    };

    class LightCulling : public RenderPass
    {
    public:
        LightCulling();
        ~LightCulling();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override {}
    private:
        void GenerateTiles(RenderPassBegin& begin);
        void CullTiles(RenderPassBegin& begin);

    private:
        uint mNumTilesX;
        uint mNumTilesY;
    };
}
