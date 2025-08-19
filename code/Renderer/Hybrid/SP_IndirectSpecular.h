//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#pragma once

#include "Renderer/SP_RenderPass.h"

#include <Graphics/Gfx_Skybox.h>

namespace SP
{
    constexpr const char* INDIRECT_SPECULAR_MASK_ID = "IndirectSpecular/HDR";
    constexpr const char* INDIRECT_SPECULAR_BAKED_CUBEMAP_ID = "IndirectSpecular/Baked";
    constexpr const char* INDIRECT_SPECULAR_BAKED_BRDF_ID = "IndirectSpecular/BRDF";

    enum class IndirectSpecularMode
    {
        kNone,
        kBaked,
        kScreenSpace,
        kRaytraced,
        kHybrid
    };

    class IndirectSpecular : public RenderPass
    {
    public:
        IndirectSpecular();
        ~IndirectSpecular();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        void None(RenderPassBegin& begin);
        void Baked(RenderPassBegin& begin);
        void ScreenSpace(RenderPassBegin& begin);
        void Raytrace(RenderPassBegin& begin);
        void Hybrid(RenderPassBegin& begin);

    private:
        IndirectSpecularMode mMode = IndirectSpecularMode::kBaked;
        
        //-- BAKED --//
        Gfx::Skybox* mCurrentSkybox = nullptr;
        bool mBakedBRDF = false;
        //----------//
    };
}
