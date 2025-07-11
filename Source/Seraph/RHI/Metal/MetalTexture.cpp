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
    : mParentDevice(device)
{
    mDesc = desc;

    MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::alloc()->init();
    descriptor->retain();

    // Set descriptor data
    descriptor->setWidth(desc.Width);
    descriptor->setHeight(desc.Height);
    descriptor->setDepth(desc.Depth);
    descriptor->setMipmapLevelCount(desc.MipLevels);
    descriptor->setPixelFormat(TranslateToMTLPixelFormat(desc.Format));

    // Set texture type
    if (desc.Depth == 6)
        descriptor->setTextureType(MTL::TextureTypeCube);
    else if (desc.Depth > 1)
        descriptor->setTextureType(MTL::TextureType3D);
    else
        descriptor->setTextureType(MTL::TextureType2D);

    // Set usage flags
    MTL::TextureUsage usage = MTL::TextureUsageShaderRead;
    if (Any(desc.Usage & RHITextureUsage::kRenderTarget)) usage |= MTL::TextureUsageRenderTarget;
    if (Any(desc.Usage & RHITextureUsage::kStorage)) usage |= MTL::TextureUsageShaderWrite;
    descriptor->setUsage(usage);
    descriptor->setStorageMode(MTL::StorageModePrivate);

    // Create texture
    mTexture = device->GetDevice()->newTexture(descriptor);
    if (!mTexture) {
        SERAPH_ERROR("Failed to create texture!");
    }
    device->GetResidencySet()->addAllocation((const MTL::Allocation*)mTexture);

    descriptor->release();
}

MetalTexture::~MetalTexture()
{
    mParentDevice->GetResidencySet()->removeAllocation((const MTL::Allocation*)mTexture);
    if (mTexture && !mDesc.Reserved) mTexture->release(); 
}

void MetalTexture::SetName(const std::string& name)
{
    mLabel = NS::String::alloc()->init(name.c_str(), NS::ASCIIStringEncoding);
    mLabel->retain();
    mTexture->setLabel(mLabel);
    mLabel->release();
}

MTL::PixelFormat MetalTexture::TranslateToMTLPixelFormat(RHITextureFormat format)
{
    switch (format)
    {
        case RHITextureFormat::kB8G8R8A8_UNORM: return MTL::PixelFormatBGRA8Unorm;
        case RHITextureFormat::kD32_FLOAT: return MTL::PixelFormatDepth32Float;
        case RHITextureFormat::kR8G8B8A8_sRGB: return MTL::PixelFormatRGBA8Unorm_sRGB;
        case RHITextureFormat::kR8G8B8A8_UNORM: return MTL::PixelFormatRGBA8Unorm;
        case RHITextureFormat::kR16G16B16A16_FLOAT: return MTL::PixelFormatRGBA16Float;
        case RHITextureFormat::kR32_FLOAT: return MTL::PixelFormatR32Float;
        case RHITextureFormat::kBC7_UNORM: return MTL::PixelFormatBC7_RGBAUnorm;
        case RHITextureFormat::kR16G16_FLOAT: return MTL::PixelFormatRG16Float;
        default: return MTL::PixelFormatInvalid;
    }
    return MTL::PixelFormatInvalid;
}
