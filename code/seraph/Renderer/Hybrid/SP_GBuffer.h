//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:46:02
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* GBUFFER_DEPTH_ID = "GBuffer/Depth";
    constexpr const char* GBUFFER_NORMAL_ID = "GBuffer/Normal";
    constexpr const char* GBUFFER_PREV_DEPTH_ID = "GBuffer/PrevDepth";
    constexpr const char* GBUFFER_PREV_NORMAL_ID = "GBuffer/PrevNormal";

    constexpr const char* GBUFFER_ALBEDO_ID = "GBuffer/Albedo";
    constexpr const char* GBUFFER_PBR_ID = "GBuffer/PBR";
    constexpr const char* GBUFFER_MOTION_VECTOR_ID = "GBuffer/MotionVector";
    constexpr const char* GBUFFER_CAMERA_CBV_ID = "GBuffer/CameraBuffer";
    constexpr const char* GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID = "GBuffer/DefaultMaterialSampler";
    constexpr const char* GBUFFER_DEFAULT_NEAREST_SAMPLER_ID = "GBuffer/DefaultNearestSampler";

    class GBuffer : public RenderPass
    {
    public:
        GBuffer();
        ~GBuffer() = default;

        void Render(RenderPassBegin& begin) override;
    private:
        void RenderScene(RenderPassBegin& begin);
        void CopyToHistory(RenderPassBegin& begin);
    };
}
