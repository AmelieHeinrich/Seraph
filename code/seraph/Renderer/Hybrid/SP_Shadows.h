//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 20:42:44
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* SHADOWS_PREVIOUS_SUN_MASK_ID = "Shadows/PrevSunMask"; // Previous noisy render
    constexpr const char* SHADOWS_SUN_MASK_LENGTH_ID = "Shadows/SunMaskLength"; // Length of the accumulated samples per pixel
    constexpr const char* SHADOWS_SUN_MASK_ID = "Shadows/SunMask"; // Denoised output
    constexpr const char* SHADOWS_CASCADE_0 = "Shadows/Cascade0";
    constexpr const char* SHADOWS_CASCADE_1 = "Shadows/Cascade1";
    constexpr const char* SHADOWS_CASCADE_2 = "Shadows/Cascade2";
    constexpr const char* SHADOWS_CASCADE_3 = "Shadows/Cascade3";

    constexpr int SHADOW_CASCADE_COUNT = 4;
    constexpr int SHADOW_CASCADE_QUALITY = 2048;

    enum class ShadowMode : uint
    {
        kNone,
        kCSM,
        kHardRT,
        kSoftRT
    };
    
    struct ShadowCascade
    {
        KGPU::BindlessHandle SRVIndex;
        float Split;
        float2 Pad;

        KGPU::float4x4 View;
        KGPU::float4x4 Proj;
    };

    class Shadows : public RenderPass
    {
    public:
        Shadows();
        ~Shadows();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        void None(RenderPassBegin& begin);

        void CSM(RenderPassBegin& begin);
        void UpdateCascades(RenderPassBegin& begin);
        void DrawCascades(RenderPassBegin& begin);
        void PopulateCSMVisibilityMask(RenderPassBegin& begin);

        void HardRT(RenderPassBegin& begin);

        void SoftRT(RenderPassBegin& begin);
        void CopyHistory(RenderPassBegin& begin);
        void TraceSoftShadowRays(RenderPassBegin& begin);
        void SVGFTemporal(RenderPassBegin& begin);
        void SVGFSpatial(RenderPassBegin& begin);

    private:
        bool mAlphaTest = true;
        ShadowMode mMode = ShadowMode::kSoftRT;

        // CSM
        KC::StaticArray<ShadowCascade, SHADOW_CASCADE_COUNT> mCascades;
        KC::StaticArray<KGPU::IBuffer*, FRAMES_IN_FLIGHT> mCascadeBuffers;
        float mSplitLambda = 0.95f;
        bool mUpdateCascades = true;

        // Hard RT 
        float mNormalBias = 0.001f;

        // Soft RT
        float mLightRadius = 0.2f;
        bool mAccumulate = true;
    };
}
