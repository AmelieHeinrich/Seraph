//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:42:04
//

#include "Renderer.h"

#include <ImGui/imgui.h>

Renderer::Renderer(IRHIDevice* device, uint width, uint height)
{
    RendererResourceManager::Initialize(device);
    RendererViewRecycler::Initialize(device);

    // Create passes
    mPathtracer = std::make_shared<Pathtracer>(device, width, height);
    mPathtracerDenoise = std::make_shared<PathtracerDenoise>(device, width, height);
    mGBuffer = std::make_shared<GBuffer>(device, width, height);
    mLightCulling = std::make_shared<LightCulling>(device, width, height);
    mDeferred = std::make_shared<Deferred>(device, width, height);
    mTonemapping = std::make_shared<Tonemapping>(device, width, height);
    mDebug = std::make_shared<Debug>(device, width, height);
    mCopyToSwapchain = std::make_shared<CopyToSwapchain>(device, width, height);

    // Setup Pathtracer
    mPasses[RenderPath::kPathtracer] = {
        mGBuffer,
        mPathtracer,
        mPathtracerDenoise,
        mTonemapping,
        mDebug,
        mCopyToSwapchain
    };

    // Setup Normal Path
    mPasses[RenderPath::kBasic] = {
        mGBuffer,
        mLightCulling,
        mDeferred,
        mTonemapping,
        mDebug,
        mCopyToSwapchain
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
        pass->Configure(path);
        pass->Render(begin);
    }
}

void Renderer::UI(RenderPath path, RenderPassBegin& begin)
{
    ImGui::Begin("Renderer Settings");
    for (auto& pass : mPasses[path]) {
        pass->UI(begin);
    }
    ImGui::End();
}
