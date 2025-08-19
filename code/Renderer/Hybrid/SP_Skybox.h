//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 15:48:51
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    class Skybox : public RenderPass
    {
    public:
        Skybox();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        bool mEnable = true;
    };
}