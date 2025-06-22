//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-21 20:57:50
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

/*
 * TODO LIST:
 * - Test for alpha cutout in simple visibility test
 * - Skybox
 * - Sample lambertian diffuse lobe for pathtracing
 * - Accumulate over time, reset when camera moves
 * - Sample specular lobe
 * - Sample light
 */

constexpr const char* PATHTRACER_HDR_TEXTURE_ID = "Pathtracer/HDR";

class Pathtracer : public RenderPass
{
public:
    Pathtracer(IRHIDevice* device, uint width, uint height);
    ~Pathtracer();

    void Render(RenderPassBegin& begin) override;
private:
    void BuildTLAS(RenderPassBegin& begin);
    void Pathtrace(RenderPassBegin& begin);

private:
    IRHIComputePipeline* mPipeline;
};
