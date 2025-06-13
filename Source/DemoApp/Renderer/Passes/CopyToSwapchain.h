//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 22:07:07
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

class CopyToSwapchain : public RenderPass
{
public:
    CopyToSwapchain(IRHIDevice* device, uint width, uint height);
    ~CopyToSwapchain();

    void Render(RenderPassBegin& begin) override;
private:
    IRHIGraphicsPipeline* mCopyPipeline;
};
