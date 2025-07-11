//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:09:00
//

#include "DummyBLAS.h"
#include "DummyDevice.h"

#undef max

DummyBLAS::DummyBLAS(DummyDevice* device, RHIBLASDesc desc)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Dummy BLAS");
}

DummyBLAS::~DummyBLAS() 
{

}

uint64 DummyBLAS::GetAddress()
{
    return 0;
}
