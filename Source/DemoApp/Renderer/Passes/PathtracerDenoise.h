//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 19:18:30
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* PATHTRACER_DENOISE_HISTORY_ID = "PathtracerDenoise/History";

class PathtracerDenoise : public RenderPass
{
public:
    PathtracerDenoise(IRHIDevice* device, uint width, uint height);
    ~PathtracerDenoise();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    void Denoise(RenderPassBegin& begin);
    void Copy(RenderPassBegin& begin);

private:
    IRHIComputePipeline* mPipeline;
};
