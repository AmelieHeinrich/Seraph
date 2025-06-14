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
        mLastViewToWorld = begin.CamData.InvView;
    }

    // Loop over all screen tiles
    for (int x = 0; x < mNumTilesX; x++) {
        for (int y = 0; y < mNumTilesY; y++) {
            LightTile& tile = mTiles[GetTileIndex(x, y)];

            // Get pixel bounds of tile
            int x0 = std::max(x * mTileDimension, 0u);
            int y0 = std::max(y * mTileDimension, 0u);
            int x1 = std::min(x0 + mTileDimension, mWidth);
            int y1 = std::min(y0 + mTileDimension, mHeight);
            
            // Generate 8 clip-space corners
            auto viewCorners = GetTileCorners({
                PixelToNDC(x0, y0),
                PixelToNDC(x1, y0),
                PixelToNDC(x0, y1),
                PixelToNDC(x1, y1)
            }, begin.CamData.InvProj);

            // Compute AABB from view corners
            glm::vec3 minCorner = viewCorners[0];
            glm::vec3 maxCorner = viewCorners[0];
            for (int i = 1; i < 8; ++i) {
                minCorner = glm::min(minCorner, viewCorners[i]);
                maxCorner = glm::max(maxCorner, viewCorners[i]);
            }

            // Store data
            tile.LightCount = 0;
            tile.Min = minCorner;
            tile.Max = maxCorner;
            for (auto& light : begin.RenderScene->GetLights().PointLights) {
                if (CheckAABBSphere(tile.Min, tile.Max, light.Position, light.Radius)) {
                    tile.LightCount++;
                }
            }

            if (mDrawTiles) {
                Debug::DrawTile(mLastViewToWorld, tile.Min, tile.Max, GetTileColor(tile.LightCount, begin.RenderScene->GetLights().PointLights.size()));
            }
        }
    }
}

void LightCulling::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Light Culling", ImGuiTreeNodeFlags_Framed)) {
        uint dimensions[] = { 16, 32, 64, 128, 256, 512 };
        const char* dimensionsStr[] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512" };

        ImGui::Checkbox("Freeze Tiles", &mFreezeTiles);
        ImGui::Checkbox("Draw Tiles", &mDrawTiles);
        ImGui::Combo("Tile Dimension", &mSelectedTileSize, dimensionsStr, 6);
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

glm::vec2 LightCulling::PixelToNDC(int x, int y)
{
    float ndcX = (2.0f * x) / mWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * y) / mHeight;
    return glm::vec2(ndcX, ndcY);
}

StaticArray<glm::vec3, 8> LightCulling::GetTileCorners(const StaticArray<glm::vec2, 4>& corners, glm::mat4 invProj)
{
    StaticArray<glm::vec3, 8> viewCorners;
    int i = 0;
    for (float z : {0.f, 1.f}) {
        for (glm::vec2 ndcXY : corners) {
            glm::vec4 clip = glm::vec4(ndcXY, z, 1.0f);
            
            glm::vec4 view = invProj * clip;
            view /= view.w;
            viewCorners[i++] = glm::vec3(view);
        }
    }
    return viewCorners;
}

bool LightCulling::CheckAABBSphere(glm::vec3 min, glm::vec3 max, glm::vec3 sphereCenter, float sphereRadius)
{
    float sqDist = 0.0f;
    for (int i = 0; i < 3; ++i) {
        float v = sphereCenter[i];
        if (v < min[i]) {
            sqDist += (min[i] - v) * (min[i] - v);
        } else if (v > max[i]) {
            sqDist += (v - max[i]) * (v - max[i]);
        }
    }
    return sqDist <= sphereRadius * sphereRadius;
}

glm::vec3 LightCulling::GetTileColor(uint tileLightCount, uint maxLightCount)
{
    float t = maxLightCount > 0 ? glm::clamp(float(tileLightCount) / float(maxLightCount), 0.0f, 1.0f) : 0.0f;

    // Blue → Green → Yellow → Red
    if (t < 0.33f) {
        // Blue to Green
        float localT = t / 0.33f;
        return glm::mix(glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), localT);
    }
    else if (t < 0.66f) {
        // Green to Yellow
        float localT = (t - 0.33f) / 0.33f;
        return glm::mix(glm::vec3(0, 1, 0), glm::vec3(1, 1, 0), localT);
    }
    else {
        // Yellow to Red
        float localT = (t - 0.66f) / 0.34f;
        return glm::mix(glm::vec3(1, 1, 0), glm::vec3(1, 0, 0), localT);
    }
}
