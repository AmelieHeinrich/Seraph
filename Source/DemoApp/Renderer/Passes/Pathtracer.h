//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:57:50
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* PATHTRACER_HDR_TEXTURE_ID = "Pathtracer/HDR";

class Pathtracer : public RenderPass
{
public:
    Pathtracer(IRHIDevice* device, uint width, uint height);
    ~Pathtracer();

    void Render(RenderPassBegin& begin) override;
private:
    IRHIComputePipeline* mPipeline;
};
