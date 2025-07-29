//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 13:57:32
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* TONEMAPPING_LDR_ID = "Tonemapping/LDR";
    constexpr const char* TONEMAPPING_SCREENSHOT_ID = "Tonemapping/Screenshot";
 
    class Tonemap : public RenderPass
    {
    public:
        Tonemap();
        ~Tonemap() = default;

        void Render(RenderPassBegin& begin) override;
    };
}
