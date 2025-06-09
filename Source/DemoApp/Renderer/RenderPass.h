//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:27:45
//

#pragma once

#include <Seraph/Seraph.h>

struct RenderPassBegin
{
    uint FrameIndex;
    IRHITexture* SwapchainTexture;
    IRHITextureView* SwapchainTextureView;
    IRHICommandList* CommandList;

    glm::mat4 View;
    glm::mat4 Projection;
};

class RenderPass
{
public:
    RenderPass(IRHIDevice* device, uint width, uint height);

    virtual void Render(RenderPassBegin& begin) = 0;
protected:
    IRHIDevice* mParentDevice;
    uint mWidth;
    uint mHeight;
};
