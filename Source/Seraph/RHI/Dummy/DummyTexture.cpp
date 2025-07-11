//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:54
//

#include "DummyTexture.h"
#include "DummyDevice.h"

#include <Core/String.h>

DummyTexture::DummyTexture(RHITextureDesc desc)
{
    mDesc = desc;
}

DummyTexture::DummyTexture(DummyDevice* device, RHITextureDesc desc)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Dummy texture");
}

DummyTexture::~DummyTexture()
{
    
}

void DummyTexture::SetName(const std::string& name)
{

}
