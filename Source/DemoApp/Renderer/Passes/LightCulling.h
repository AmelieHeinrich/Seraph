//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:55:27
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

struct LightTile
{
    glm::vec3 Min;
    glm::vec3 Max;
    uint LightCount;
};

class LightCulling : public RenderPass
{
public:
    LightCulling(IRHIDevice* device, uint width, uint height);
    ~LightCulling();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    glm::vec2 PixelToNDC(int x, int y);
    StaticArray<glm::vec3, 8> GetTileCorners(const StaticArray<glm::vec2, 4>& corners, glm::mat4 invProj);
    bool CheckAABBSphere(glm::vec3 min, glm::vec3 max, glm::vec3 sphereCenter, float sphereRadius);
    glm::vec3 GetTileColor(uint tileLightCount, uint maxLightCount);

    inline int GetTileIndex(int x, int y) const
    {
        return y * mNumTilesX + x;
    }

    uint mTileDimension = 16;
    uint mNumTilesX;
    uint mNumTilesY;
    Array<LightTile> mTiles;

private:
    // Settings
    bool mFreezeTiles = false;
    bool mDrawTiles = true;
    int mSelectedTileSize = 0;
    glm::mat4 mLastViewToWorld;
};
