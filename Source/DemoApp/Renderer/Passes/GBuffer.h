//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:50:26
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* GBUFFER_DEPTH_ID = "GBuffer/Depth";
constexpr const char* GBUFFER_NORMAL_ID = "GBuffer/Normal";
constexpr const char* GBUFFER_ALBEDO_ID = "GBuffer/Albedo";
constexpr const char* GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID = "GBuffer/DefaultMaterialSampler";
constexpr const char* GBUFFER_DEFAULT_NEAREST_SAMPLER_ID = "GBuffer/DefaultNearestSampler";

class GBuffer : public RenderPass
{
public:
    GBuffer(IRHIDevice* device, uint width, uint height);
    ~GBuffer();

    void Render(RenderPassBegin& begin) override;
private:
    IRHIGraphicsPipeline* mPipeline;
};
