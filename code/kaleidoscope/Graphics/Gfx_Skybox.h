//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 15:09:16
//

#pragma once

#include "Gfx_Manager.h"

namespace Gfx
{
    struct Skybox
    {
        KC::String Path;

        KGPU::ITexture* EnvironmentMap;
        KGPU::ITextureView* CubeView;

        void Clean()
        {
            if (EnvironmentMap) KC_DELETE(EnvironmentMap);
            if (CubeView) KC_DELETE(CubeView);
        }
    };

    class SkyboxCooker
    {
    public:
        static void Initialize();
        static void GenerateSkybox(Skybox& sky, const KC::String& path);
    };
}
