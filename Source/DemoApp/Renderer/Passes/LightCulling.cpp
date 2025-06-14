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
    mNumTilesX = (width + mTileWidth - 1) / mTileWidth;
    mNumTilesY = (height + mTileHeight - 1) / mTileHeight;
    mTiles.resize(mNumTilesX * mNumTilesY);
}

LightCulling::~LightCulling()
{

}

void LightCulling::Render(RenderPassBegin& begin)
{
    if (!mFreezeTiles) {
        mLastViewToWorld = begin.CamData.InvView;
        mLastView = begin.CamData.View;
    }

    for (int x = 0; x < mNumTilesX; x++) {
        for (int y = 0; y < mNumTilesY; y++) {
            LightTile& tile = mTiles[GetTileIndex(x, y)];

            int x0 = std::max(x * mTileWidth, 0u);
            int y0 = std::max(y * mTileHeight, 0u);
            int x1 = std::min(x0 + mTileWidth, mWidth);
            int y1 = std::min(y0 + mTileHeight, mHeight);

            glm::vec2 ndcTL = PixelToNDC(x0, y0);
            glm::vec2 ndcTR = PixelToNDC(x1, y0);
            glm::vec2 ndcBR = PixelToNDC(x1, y1);
            glm::vec2 ndcBL = PixelToNDC(x0, y1);

            glm::vec3 v0 = glm::vec3(NDCToView(ndcTL, 0.0f, begin.CamData.InvProj));
            glm::vec3 v1 = glm::vec3(NDCToView(ndcTR, 0.0f, begin.CamData.InvProj));
            glm::vec3 v2 = glm::vec3(NDCToView(ndcBR, 0.0f, begin.CamData.InvProj));
            glm::vec3 v3 = glm::vec3(NDCToView(ndcBL, 0.0f, begin.CamData.InvProj));

            glm::vec3 v4 = glm::vec3(NDCToView(ndcTL, 1.0f, begin.CamData.InvProj));
            glm::vec3 v5 = glm::vec3(NDCToView(ndcTR, 1.0f, begin.CamData.InvProj));
            glm::vec3 v6 = glm::vec3(NDCToView(ndcBR, 1.0f, begin.CamData.InvProj));
            glm::vec3 v7 = glm::vec3(NDCToView(ndcBL, 1.0f, begin.CamData.InvProj));

            // Correct CCW winding from inside the frustum
            FrustumPlane left   = MakePlane(v0, v3, v7);
            FrustumPlane right  = MakePlane(v2, v1, v5);
            FrustumPlane top    = MakePlane(v1, v0, v4);
            FrustumPlane bottom = MakePlane(v3, v2, v6);
            FrustumPlane near   = MakePlane(v0, v1, v2);
            FrustumPlane far    = MakePlane(v5, v4, v7);

            StaticArray<FrustumPlane, 6> planes = { left, right, top, bottom, near, far };

            // Count visible lights
            tile.LightCount = 0;
            for (auto& light : begin.RenderScene->GetLights().PointLights) {
                glm::vec3 viewPos = glm::vec3(mLastView * glm::vec4(light.Position, 1.0f));
                bool inside = SphereFrustumTest(viewPos, light.Radius, planes);
                if (inside) {
                    tile.LightCount++;
                }
            
                if ((mDrawLightInTiles || mDrawLightChecks) && (mDrawTiles || (mDrawSpecificTile && mSelectedTileX == x && mSelectedTileY == y))) {
                    glm::vec3 color = inside ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
                    if (inside) {
                        Debug::DrawRings(light.Position, light.Radius, color);
                    } else {
                        if (mDrawLightChecks) {
                            Debug::DrawRings(light.Position, light.Radius, color);
                        }
                    }
                }
            }

            bool drawThisTile =
                mDrawTiles ||
                (mDrawSpecificTile && mSelectedTileX == x && mSelectedTileY == y);

            if (drawThisTile) {
                // Color-coded frustum debug
                glm::vec3 color = GetTileColor(tile.LightCount, begin.RenderScene->GetLights().PointLights.size());
                Debug::DrawFrustumCorners(mLastViewToWorld, {
                    v0, v1, v2, v3, v4, v5, v6, v7
                }, color);
            }
        }
    }
}

glm::vec2 LightCulling::PixelToNDC(int x, int y)
{
    float ndcX = (2.0f * x) / mWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * y) / mHeight;
    return glm::vec2(ndcX, ndcY);
}

glm::vec4 LightCulling::NDCToView(glm::vec2 ndc, float z, glm::mat4 invProj)
{
    glm::vec4 clip = glm::vec4(ndc, z, 1.0f);
    glm::vec4 view = invProj * clip;
    return view / view.w;
}

FrustumPlane LightCulling::MakePlane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    // Ensure correct winding (CCW from inside the frustum)
    glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
    float d = -glm::dot(normal, a);

    return { normal, d };
}

bool LightCulling::SphereFrustumTest(glm::vec3 center, float radius, const StaticArray<FrustumPlane, 6>& planes)
{
    for (const auto& plane : planes) {
        float distance = glm::dot(plane.Normal, center) + plane.Distance;
        if (distance < -radius) return false; // Outside this plane
    }
    return true;
}

void LightCulling::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Light Culling", ImGuiTreeNodeFlags_Framed)) {
        uint widths[] = { 16, 32, 64, 128, 256, 512, 1920 };
        uint height[] = { 16, 32, 64, 128, 256, 512, 1080 };
        const char* dimensionsStr[] = { "16x16", "32x32", "64x64", "128x128", "256x256", "512x512", "1920x1080" };

        int inputX = mSelectedTileX;
        int inputY = mSelectedTileY;

        ImGui::Checkbox("Freeze Tiles", &mFreezeTiles);
        ImGui::Combo("Tile Dimension", &mSelectedTileSize, dimensionsStr, 7);
        if (ImGui::TreeNodeEx("Debug Draw", ImGuiTreeNodeFlags_Framed)) {
            ImGui::Checkbox("Draw Tiles", &mDrawTiles);
            ImGui::Checkbox("Draw Selected Tile", &mDrawSpecificTile);
            ImGui::InputInt("Selected Tile X", &inputX);
            ImGui::InputInt("Selected Tile Y", &inputY);
            ImGui::Checkbox("Draw Point Light Checks", &mDrawLightChecks);
            ImGui::Checkbox("Draw Point Lights in Tile", &mDrawLightInTiles);
            ImGui::TreePop();
        }
        ImGui::TreePop();

        if (inputX < mNumTilesX && inputX >= 0) mSelectedTileX = inputX;
        if (inputY < mNumTilesY && inputY >= 0) mSelectedTileY = inputY;

        if (widths[mSelectedTileSize] != mTileWidth || height[mSelectedTileSize] != mTileHeight) {
            mTileWidth = widths[mSelectedTileSize];
            mTileHeight = widths[mSelectedTileSize];
            mNumTilesX = (mWidth + mTileWidth - 1) / mTileWidth;
            mNumTilesY = (mHeight + mTileHeight - 1) / mTileHeight;
            mTiles.clear();
            mTiles.resize(mNumTilesX * mNumTilesY);
        }
    }
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
