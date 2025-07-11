//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:10:22
//

#include "DummyTLAS.h"
#include "DummyDevice.h"

DummyTLAS::DummyTLAS(DummyDevice* device)
    : mParentDevice(device)
{
    SERAPH_WHATEVER("Created Dummy TLAS");
}

DummyTLAS::~DummyTLAS()
{
    
}
