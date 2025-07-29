//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:58:44
//

#include "KGPU_DummyTexture.h"
#include "KGPU_DummyDevice.h"

namespace KGPU
{
    DummyTexture::DummyTexture(TextureDesc desc)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }
    }
    
    DummyTexture::DummyTexture(DummyDevice* device, TextureDesc desc)
    {
        mDesc = desc;
        for (int i = 0; i < desc.MipLevels; i++) {
            mLayouts.push_back(KGPU::ResourceLayout::kUndefined);
        }
        
        KD_WHATEVER("Created Dummy texture");
    }
    
    DummyTexture::~DummyTexture()
    {
    }
    
    void DummyTexture::SetName(const KC::String& name)
    {
    }
}
