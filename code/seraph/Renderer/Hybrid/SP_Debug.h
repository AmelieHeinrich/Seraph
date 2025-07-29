//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 21:26:43
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    class Debug : public RenderPass
    {
    public:
        Debug();
        ~Debug() = default;

        void Render(RenderPassBegin& begin) override;
    };
}
