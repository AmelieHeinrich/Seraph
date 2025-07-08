//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-08 22:57:28
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

class TLASPrepare : public RenderPass
{
public:
    TLASPrepare(IRHIDevice* device, uint width, uint height);
    ~TLASPrepare();

    void Render(RenderPassBegin& begin) override;
};
