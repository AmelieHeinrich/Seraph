//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:27:57
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* SHADOWS_SUN_MASK_ID = "Shadows/SunMask";

enum class ShadowMode : uint
{
    kNone,
    kCSM,
    kHardRT,
    kSoftRT
};

class Shadows : public RenderPass
{
public:
    Shadows(IRHIDevice* device, uint width, uint height);
    ~Shadows();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    void None(RenderPassBegin& begin);
    void HardRT(RenderPassBegin& begin);

private:
    bool mAlphaTest = true;
    ShadowMode mMode = ShadowMode::kHardRT;

    IRHIComputePipeline* mHardRTShadows;
    IRHIComputePipeline* mHardRTShadowsNoAlpha;
    float mNormalBias = 0.001f;
};
