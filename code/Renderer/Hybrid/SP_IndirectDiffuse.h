//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:55:26
//

#pragma once

#include "Renderer/SP_RenderPass.h"

namespace SP
{
    constexpr const char* INDIRECT_DIFFUSE_MASK_ID = "IndirectDiffuse/HDR";

    enum class IndirectDiffuseMode
    {
        kNone,
        kConstantAmbient,
        kSSGI,
        kDDGI
    };

    class IndirectDiffuse : public RenderPass
    {
    public:
        IndirectDiffuse();
        ~IndirectDiffuse();

        void Render(RenderPassBegin& begin) override;
        void UI(RenderPassBegin& begin) override;
    private:
        void None(RenderPassBegin& begin);
        void ConstantAmbient(RenderPassBegin& begin);
        void SSGI(RenderPassBegin& begin);
        void DDGI(RenderPassBegin& begin);

    private:
        IndirectDiffuseMode mMode = IndirectDiffuseMode::kNone;
        glm::vec3 mConstantAmbient = glm::vec3(0.1f);
    };
}
