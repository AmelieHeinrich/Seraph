//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-14 20:24:04
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

class VisualizeMotionVectors : public RenderPass
{
public:
    VisualizeMotionVectors(IRHIDevice* device, uint width, uint height);
    ~VisualizeMotionVectors();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    bool mEnable = true;
};
