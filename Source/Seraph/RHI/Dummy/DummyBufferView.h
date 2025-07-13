//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:11:15
//

#pragma once

#include <RHI/BufferView.h>

class DummyDevice;

class DummyBufferView : public IRHIBufferView
{
public:
    DummyBufferView(DummyDevice* device, RHIBufferViewDesc desc);
    ~DummyBufferView() override;

private:
    DummyDevice* mParentDevice;
};
