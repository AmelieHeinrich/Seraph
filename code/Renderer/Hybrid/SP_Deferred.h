//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* DEFERRED_HDR_TEXTURE_ID = "Deferred/HDR";

    class Deferred : public RenderPass
    {
    public:
        Deferred();
        ~Deferred();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        bool mShowTileHeatmap = false;
    };
}
