//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:35:05
//

#pragma once

#include "RenderPass.h"

#include "Passes/Pathtracer.h"
#include "Passes/PathtracerDenoise.h"
#include "Passes/LightCulling.h"
#include "Passes/GBuffer.h"
#include "Passes/Deferred.h"
#include "Passes/Tonemapping.h"
#include "Passes/Debug.h"
#include "Passes/CopyToSwapchain.h"

class Renderer
{
public:
    Renderer(IRHIDevice* device, uint width, uint height);
    ~Renderer();

    void Render(RenderPath path, RenderPassBegin& begin);
    void UI(RenderPath path, RenderPassBegin& begin);
private:
    SharedPtr<Pathtracer> mPathtracer;
    SharedPtr<PathtracerDenoise> mPathtracerDenoise;
    SharedPtr<GBuffer> mGBuffer;
    SharedPtr<LightCulling> mLightCulling;
    SharedPtr<Deferred> mDeferred;
    SharedPtr<Tonemapping> mTonemapping;
    SharedPtr<Debug> mDebug;
    SharedPtr<CopyToSwapchain> mCopyToSwapchain;

    UnorderedMap<RenderPath, Array<SharedPtr<RenderPass>>> mPasses;
};
