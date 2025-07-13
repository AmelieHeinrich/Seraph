//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:09:52
//

#pragma once

#include <RHI/TLAS.h>
#include <RHI/Buffer.h>

class DummyDevice;

class DummyTLAS : public IRHITLAS
{
public:
    DummyTLAS(DummyDevice* device);
    ~DummyTLAS() override;

    uint64 Address() const { return mMemory->GetAddress(); }
private:
    friend class DummyCommandList;

    DummyDevice* mParentDevice;
};
