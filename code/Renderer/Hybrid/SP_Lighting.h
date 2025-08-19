//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-08-19 11:54:00
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* LIGHTING_OUTPUT_ID = "Lighting/Output";

    class Lighting : public RenderPass
    {
    public:
        Lighting();
        ~Lighting();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    };
}
