//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:27:57
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* SHADOWS_SUN_MASK_ID = "Shadows/SunMask";
constexpr const char* SHADOWS_CASCADE_0 = "Shadows/Cascade0";
constexpr const char* SHADOWS_CASCADE_1 = "Shadows/Cascade1";
constexpr const char* SHADOWS_CASCADE_2 = "Shadows/Cascade2";
constexpr const char* SHADOWS_CASCADE_3 = "Shadows/Cascade3";

constexpr int SHADOW_CASCADE_COUNT = 4;
constexpr int SHADOW_CASCADE_QUALITY = 4096;

enum class ShadowMode : uint
{
    kNone,
    kCSM,
    kHardRT,
    kSoftRT
};

struct ShadowCascade
{
    BindlessHandle SRVIndex;
    float Split;
    float2 Pad;

    glm::mat4 View;
    glm::mat4 Proj;
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
    void CSM(RenderPassBegin& begin);
    void HardRT(RenderPassBegin& begin);

private:
    bool mAlphaTest = true;
    ShadowMode mMode = ShadowMode::kHardRT;

    // CSM
    IRHIGraphicsPipeline* mCSMPipeline;
    IRHIGraphicsPipeline* mCSMPipelineNoAlpha;
    IRHIComputePipeline* mCSMToMaskPipeline;
    StaticArray<ShadowCascade, SHADOW_CASCADE_COUNT> mCascades;
    StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> mCascadeBuffers;
    float mSplitLambda = 0.95f;
    bool mUpdateCascades = true;

    // Hard RT 
    IRHIComputePipeline* mHardRTShadows;
    IRHIComputePipeline* mHardRTShadowsNoAlpha;
    float mNormalBias = 0.001f;
};
