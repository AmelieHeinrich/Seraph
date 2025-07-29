//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 22:33:10
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    class MotionVector : public RenderPass
    {
    public:
        MotionVector();
        ~MotionVector() = default;

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        bool mEnable = false;
    };
}
