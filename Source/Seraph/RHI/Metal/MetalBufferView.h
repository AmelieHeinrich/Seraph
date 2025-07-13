//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:15
//

#pragma once

#include <RHI/BufferView.h>

class MetalDevice;

class MetalBufferView : public IRHIBufferView
{
public:
    MetalBufferView(MetalDevice* device, RHIBufferViewDesc desc);
    ~MetalBufferView() override;

private:
    MetalDevice* mParentDevice;
};
