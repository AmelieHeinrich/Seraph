//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:35:05
//

#pragma once

#include "RenderPass.h"

enum class RenderPath
{
    kBasic,
    kPathtracer
};

class Renderer
{
public:
    Renderer(IRHIDevice* device, uint width, uint height);
    ~Renderer();

    void Render(RenderPath path, RenderPassBegin& begin);
private:
    UnorderedMap<RenderPath, Array<RenderPass*>> mPasses;
};
