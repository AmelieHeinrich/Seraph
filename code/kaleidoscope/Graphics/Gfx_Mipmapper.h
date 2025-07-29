//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 23:16:20
//

#pragma once

#include "Gfx_Manager.h"

namespace Gfx
{
    class Mipmapper
    {
    public:
        static void Initialize();

        static void ComputeMipmaps(KGPU::ITexture* texture);
    };
}
