//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:42:04
//

#include "Renderer.h"

#include "Passes/GBuffer.h"
#include "Passes/Deferred.h"
#include "Passes/Tonemapping.h"

Renderer::Renderer(IRHIDevice* device, uint width, uint height)
{
    RendererResourceManager::Initialize(device);
    RendererViewRecycler::Initialize(device);

    // Setup Pathtracer

    // Setup Normal Path
    mPasses[RenderPath::kBasic] = {
        new GBuffer(device, width, height),
        new Deferred(device, width, height),
        new Tonemapping(device, width, height)  
    };
}

Renderer::~Renderer()
{
    for (auto& pass : mPasses[RenderPath::kBasic]) {
        delete pass;
    }
    for (auto& pass : mPasses[RenderPath::kPathtracer]) {
        delete pass;
    }
    mPasses.clear();

    RendererResourceManager::Shutdown();
    RendererViewRecycler::Shutdown();
}

void Renderer::Render(RenderPath path, RenderPassBegin& begin)
{
    for (auto& pass : mPasses[path]) {
        pass->Render(begin);
    }
}
