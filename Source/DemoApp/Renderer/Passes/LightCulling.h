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
    glm::vec4 NDCToView(glm::vec2 ndc, float z, glm::mat4 invProj);
    FrustumPlane MakePlane(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    bool SphereFrustumTest(glm::vec3 center, float radius, const StaticArray<FrustumPlane, 6>& planes);
    glm::vec3 GetTileColor(uint tileLightCount, uint maxLightCount);

    inline int GetTileIndex(int x, int y) const
    {
        return y * mNumTilesX + x;
    }

    uint mTileWidth = 16;
    uint mTileHeight = 16;
    uint mNumTilesX;
    uint mNumTilesY;
    Array<LightTile> mTiles;

private:
    // Settings
    bool mFreezeTiles = false;
    bool mDrawTiles = false;
    bool mDrawSpecificTile = false;
    bool mDrawLightChecks = false;
    bool mDrawLightInTiles = false;
    int mSelectedTileX = 0;
    int mSelectedTileY = 0;
    int mSelectedTileSize = 0;
    glm::mat4 mLastViewToWorld;
    glm::mat4 mLastView;
};
