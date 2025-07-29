//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:55:37
//

#pragma once

#include "SP_RenderPass.h"

namespace SP
{
    class WorldRenderer
    {
    public:
        WorldRenderer();
        ~WorldRenderer();

        void Render(RenderPassBegin& begin);
        void UI(RenderPassBegin& begin);
        void Prepare();
    private:
        KC::Array<RenderPass*> mPasses;
    };
}
