//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:36:36
//

#pragma once

#include <KernelGPU/KGPU_BLAS.h>

namespace KGPU
{
    class DummyDevice;

    class DummyBLAS : public IBLAS
    {
    public:
        DummyBLAS(DummyDevice* device, BLASDesc desc);
        ~DummyBLAS();

        uint64 GetAddress() override;
    };
}
