//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* AMBIENT_OCCLUSION_MASK_ID = "AmbientOcclusion/Mask";

    enum class AmbientOcclusionMode
    {
        kNone,
        kSSAO,
        kGTAO,
        kRTAO
    };

    class AmbientOcclusion : public RenderPass
    {
    public:
        AmbientOcclusion();
        ~AmbientOcclusion();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        void None(RenderPassBegin& begin);
        void SSAO(RenderPassBegin& begin);
        void GTAO(RenderPassBegin& begin);
        void RTAO(RenderPassBegin& begin);

    private:
        AmbientOcclusionMode mMode = AmbientOcclusionMode::kNone;
    };
}
