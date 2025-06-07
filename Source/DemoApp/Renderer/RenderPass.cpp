//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:34:13
//

#include "RenderPass.h"

RenderPass::RenderPass(IRHIDevice* device, uint width, uint height)
    : mParentDevice(device), mWidth(width), mHeight(height)
{
}
