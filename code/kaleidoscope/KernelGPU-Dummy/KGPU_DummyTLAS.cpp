//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:06:13
//

#include "KGPU_DummyTLAS.h"
#include "KGPU_DummyDevice.h"

#undef max

namespace KGPU
{
    DummyTLAS::DummyTLAS(DummyDevice* device)
    {
        KD_WHATEVER("Created Dummy TLAS");
    }
    
    DummyTLAS::~DummyTLAS()
    {
    }
}
