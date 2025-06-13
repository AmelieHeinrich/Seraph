//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:27:45
//

#pragma once

#include <Seraph/Seraph.h>

struct CameraData
{
    glm::mat4 View;
    glm::mat4 Proj;
    glm::mat4 ViewProj;
    glm::mat4 InvView;
    glm::mat4 InvProj;
    glm::mat4 InvViewProj;
    glm::vec4 Position;
};

struct RenderPassBegin
{
    uint FrameIndex;
    IRHITexture* SwapchainTexture;
    IRHITextureView* SwapchainTextureView;
    IRHICommandList* CommandList;
    Scene* RenderScene;

    CameraData CamData;
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
