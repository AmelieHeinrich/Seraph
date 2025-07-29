//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 14:09:37
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    class Composite : public RenderPass
    {
    public:
        Composite();
        ~Composite() = default;

        void Render(RenderPassBegin& begin) override;
    };
}
