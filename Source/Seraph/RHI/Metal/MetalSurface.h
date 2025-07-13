//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:46:25
//

#pragma once

#include <RHI/Surface.h>
#include <MetalCPP/Metal/Metal.hpp>
#include <MetalCPP/QuartzCore/QuartzCore.hpp>

#include "MetalTexture.h"

class MetalDevice;
class MetalCommandQueue;

class MetalSurface : public IRHISurface
{
public:
    MetalSurface(MetalDevice* device, Window* window, MetalCommandQueue* commandQueue);
    ~MetalSurface() override;
    
public:
    CA::MetalLayer* GetLayer() { return mLayer; }

private:
    MetalDevice* mParentDevice;

    SDL_MetalView mView;
    CA::MetalLayer* mLayer;
};
