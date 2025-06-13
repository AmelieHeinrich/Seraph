//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:53:29
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* TONEMAPPING_LDR_ID = "Tonemapping/LDR";

class Tonemapping : public RenderPass
{
public:
    Tonemapping(IRHIDevice* device, uint width, uint height);
    ~Tonemapping();

    void Render(RenderPassBegin& begin) override;
private:
    void Tonemap(RenderPassBegin& begin);
    void Copy(RenderPassBegin& begin);

    IRHIComputePipeline* mPipeline;
    IRHIGraphicsPipeline* mResolvePipeline;
};
