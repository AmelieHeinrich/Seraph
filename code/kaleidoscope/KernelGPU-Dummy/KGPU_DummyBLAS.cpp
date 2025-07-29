//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:37:46
//

#include "KGPU_DummyBLAS.h"
#include "KGPU_DummyDevice.h"

#undef max

namespace KGPU
{
    DummyBLAS::DummyBLAS(DummyDevice* device, BLASDesc desc)
    {
        mDesc = desc;

        KD_WHATEVER("Created Dummy BLAS");
    }
    
    DummyBLAS::~DummyBLAS() 
    {

    }
    
    uint64 DummyBLAS::GetAddress()
    {
        return 0;
    }
}
