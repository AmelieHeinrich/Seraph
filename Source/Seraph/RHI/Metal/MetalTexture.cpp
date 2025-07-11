//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:50:54
//

#include "MetalTexture.h"
#include "MetalDevice.h"

#include <Core/String.h>

MetalTexture::MetalTexture(RHITextureDesc desc)
{
    mDesc = desc;
}

MetalTexture::MetalTexture(MetalDevice* device, RHITextureDesc desc)
{
    mDesc = desc;

    SERAPH_WHATEVER("Created Metal texture");
}

MetalTexture::~MetalTexture()
{
    
}

void MetalTexture::SetName(const std::string& name)
{

}
