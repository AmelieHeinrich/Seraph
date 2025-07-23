//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 20:03:14
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* REFLECTIONS_CHANNEL_ID = "Reflections/Texture";
constexpr const char* REFLECTIONS_LIGHTING_COPY_ID = "Reflections/LightingCopyID";

enum class ReflectionsMode : uint
{
    kNone,
    kScreenSpace,
    kRayTrace,
    kHybrid
};

class Reflections : public RenderPass
{
public:
    Reflections(IRHIDevice* device, uint width, uint height);
    ~Reflections();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    void None(RenderPassBegin& begin);
    void ScreenSpace(RenderPassBegin& begin);
    void RayTrace(RenderPassBegin& begin);
    void Hybrid(RenderPassBegin& begin);

private:
    bool mAlphaTest = true;
    ReflectionsMode mMode = ReflectionsMode::kNone;
};
