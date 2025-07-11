//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:09:00
//

#include "MetalBLAS.h"
#include "MetalDevice.h"

#undef max

MetalBLAS::MetalBLAS(MetalDevice* device, RHIBLASDesc desc)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Metal BLAS");
}

MetalBLAS::~MetalBLAS() 
{
    
}

uint64 MetalBLAS::GetAddress()
{
    return 0;
}
