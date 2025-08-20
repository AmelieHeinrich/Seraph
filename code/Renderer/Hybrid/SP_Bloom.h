//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-08-20 14:16:00
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* BLOOM_TEXTURE_ID = "Bloom/Mask";
    constexpr const char* BLOOM_LINEAR_CLAMP_SAMPLER_ID = "Bloom/LinearClampSampler";
    constexpr const char* BLOOM_LINEAR_BORDER_SAMPLER_ID = "Bloom/LinearBorderSampler";
    constexpr const char* BLOOM_POINT_CLAMP_SAMPLER_ID = "Bloom/PointClampSampler";
    
    class Bloom : public RenderPass
    {
    public:
        Bloom();
        ~Bloom();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;

    private:
        bool mEnable = true;
        float mUpsampleGain = 0.4f;
        float mThreshold = 20.0f;
        float mKnee = 10.0;
        float mStrength = 5.0f;
        bool mOnlyEmissive = false;

        static constexpr uint BLOOM_MIP_CHAIN = 8;
    };
}
