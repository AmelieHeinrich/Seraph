//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* RADIANCE_HDR_TEXTURE_ID = "Radiance/HDR";

    enum class RadianceMode
    {
        kRasterized,
        kRaytraced,
        kRaytracedReSTIR
    };

    class Radiance : public RenderPass
    {
    public:
        Radiance();
        ~Radiance();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        void Rasterize(RenderPassBegin& begin);
        void Raytrace(RenderPassBegin& begin);
        void RaytraceReSTIR(RenderPassBegin& begin);

    private:
        bool mShowTileHeatmap = false;
        RadianceMode mMode = RadianceMode::kRasterized;
    };
}
