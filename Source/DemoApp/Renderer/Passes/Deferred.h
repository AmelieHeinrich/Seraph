//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:52:34
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* DEFERRED_HDR_TEXTURE_ID = "Deferred/HDR";

class Deferred : public RenderPass
{
public:
    Deferred(IRHIDevice* device, uint width, uint height);
    ~Deferred();

    void Render(RenderPassBegin& begin) override;
private:
    IRHIComputePipeline* mPipeline;
};
