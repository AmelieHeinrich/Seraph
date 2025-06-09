//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:42:04
//

#include "Renderer.h"

#include "Passes/GBuffer.h"
#include "Passes/Deferred.h"
#include "Passes/Tonemapping.h"
#include "Passes/Debug.h"

Renderer::Renderer(IRHIDevice* device, uint width, uint height)
{
    RendererResourceManager::Initialize(device);
    RendererViewRecycler::Initialize(device);

    // Setup Pathtracer

    // Setup Normal Path
    mPasses[RenderPath::kBasic] = {
        std::make_shared<GBuffer>(device, width, height),
        std::make_shared<Deferred>(device, width, height),
        std::make_shared<Tonemapping>(device, width, height),
        std::make_shared<Debug>(device, width, height)
    };
}

Renderer::~Renderer()
{
    for (auto& passes : mPasses) {
        passes.second.clear();
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
