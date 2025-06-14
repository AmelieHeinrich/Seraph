//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:56:07
//

#include "LightCulling.h"
#include "Debug.h"

#include <ImGui/imgui.h>

LightCulling::LightCulling(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    mNumTilesX = (width + mTileDimension - 1) / mTileDimension;
    mNumTilesY = (height + mTileDimension - 1) / mTileDimension;
    mTiles.resize(mNumTilesX * mNumTilesY);
}

LightCulling::~LightCulling()
{

}

void LightCulling::Render(RenderPassBegin& begin)
{
    if (!mFreezeTiles) {
        mLastInvProj = begin.CamData.InvProj;
        mLastViewToWorld = begin.CamData.InvView;
    }

    auto PixelToNDC = [&](int x, int y) -> glm::vec2 {
        // Clamp to screen bounds first
        x = std::clamp(x, 0, (int)mWidth - 1);
        y = std::clamp(y, 0, (int)mHeight - 1);
        
        float ndcX = (2.0f * x) / mWidth - 1.0f;
        float ndcY = (2.0f * y) / mHeight - 1.0f;
        return glm::vec2(ndcX, ndcY);
    };

    // Loop over all screen tiles
    for (int x = 0; x < mNumTilesX; x++) {
        for (int y = 0; y < mNumTilesY; y++) {
            int tileIndex = GetTileIndex(x, y);
            LightTile& tile = mTiles[tileIndex];

            // Get pixel bounds of tile
            int x0 = std::max(x * mTileDimension, 0u);
            int y0 = std::max(y * mTileDimension, 0u);
            int x1 = std::min(x0 + mTileDimension, mWidth);
            int y1 = std::min(y0 + mTileDimension, mHeight);

            // Convert to NDC
            glm::vec2 ndcTL = PixelToNDC(x0, y0);
            glm::vec2 ndcTR = PixelToNDC(x1, y0);
            glm::vec2 ndcBL = PixelToNDC(x0, y1);
            glm::vec2 ndcBR = PixelToNDC(x1, y1);
            
            // Generate 8 clip-space corners
            StaticArray<glm::vec3, 8> viewCorners;
            int i = 0;
            for (float z : {0.0f, 1.0f}) {
                for (glm::vec2 ndcXY : {ndcTL, ndcTR, ndcBR, ndcBL}) {
                    glm::vec4 clip = glm::vec4(ndcXY, z, 1.0f);
                    glm::vec4 view = mLastInvProj * clip;
                    view /= view.w;
                    viewCorners[i++] = glm::vec3(view);
                }
            }

            // Compute AABB from view corners
            glm::vec3 minCorner = viewCorners[0];
            glm::vec3 maxCorner = viewCorners[0];

            for (int i = 1; i < 8; ++i) {
                minCorner = glm::min(minCorner, viewCorners[i]);
                maxCorner = glm::max(maxCorner, viewCorners[i]);
            }

            // Store data
            tile.Min = minCorner;
            tile.Max = maxCorner;
            tile.LightCount = 0;

            if (mDrawTiles) {
                Debug::DrawBox(mLastViewToWorld, tile.Min, tile.Max);
            }
        }
    }
}

void LightCulling::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Light Culling", ImGuiTreeNodeFlags_Framed)) {
        uint dimensions[] = { 8, 16, 32, 64, 128, 256, 512 };
        const char* dimensionsStr[] = { "8x8", "16x16", "32x32", "64x64", "128x128", "256x256", "512x512" };

        ImGui::Checkbox("Freeze Tiles", &mFreezeTiles);
        ImGui::Checkbox("Draw Tiles", &mDrawTiles);
        ImGui::Combo("Tile Dimension", &mSelectedTileSize, dimensionsStr, 7);
        ImGui::TreePop();

        if (dimensions[mSelectedTileSize] != mTileDimension) {
            mTileDimension = dimensions[mSelectedTileSize];
            mNumTilesX = (mWidth + mTileDimension - 1) / mTileDimension;
            mNumTilesY = (mHeight + mTileDimension - 1) / mTileDimension;
            mTiles.clear();
            mTiles.resize(mNumTilesX * mNumTilesY);
        }
    }
}
