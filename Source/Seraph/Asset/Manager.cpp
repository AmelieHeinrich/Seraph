//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-09 12:20:56
//

#include "Manager.h"
#include "Compressor.h"

#include <Core/FileSystem.h>
#include <RHI/Uploader.h>

AssetManager::Data AssetManager::sData;

Asset::~Asset()
{
    switch (Type) {
        case AssetType::kModel: {
            delete Model;
            break;
        }
        case AssetType::kImage:
        case AssetType::kTexture: {
            delete TextureOrImage.View;
            delete TextureOrImage.Handle;
            break;
        }
    }
}

void AssetManager::Initialize(IRHIDevice* device)
{
    sData.Device = device;
    sData.Assets.clear();
}

void AssetManager::Shutdown()
{
    for (auto& asset : sData.Assets) {
        Destroy(asset.second);
    }
    sData.Assets.clear();
}

void AssetManager::Update()
{
    if (sData.Assets.empty())
        return;
    for (auto it = sData.Assets.begin(); it != sData.Assets.end(); ) {
        if (!FileSystem::Exists(it->first)) {
            delete it->second;
            it = sData.Assets.erase(it);
        } else {
            ++it;
        }
    }
}

Asset::Handle AssetManager::Get(const String& path, AssetType type)
{
    if (!FileSystem::Exists(path))
        return nullptr;
    
    if (sData.Assets.count(path) > 0) {
        sData.Assets[path]->RefCount++;
        return sData.Assets[path];
    }

    Asset::Handle handle = new Asset;
    handle->RefCount = true;
    handle->Type = type;
    handle->Path = path;

    switch (type) {
        case AssetType::kModel: {
            handle->Model = new Model(sData.Device, path);
            SERAPH_INFO("Loading model %s", path.c_str());
            break;
        }
        case AssetType::kTexture: {
            if (!FileSystem::Exists(Compressor::ToCachedPath(path))) {
                SERAPH_ERROR("Asset %s isn't compressed and thus can't be loaded as a texture!", path.c_str());
                return nullptr;
            }

            TextureAsset asset;
            asset.Load(Compressor::ToCachedPath(path));

            RHITextureDesc desc = {};
            desc.Width = asset.Header.Width;
            desc.Height = asset.Header.Height;
            desc.MipLevels = asset.Header.Mips - 2;
            desc.Format = asset.Header.Format;
            desc.Usage = RHITextureUsage::kShaderResource;
            
            handle->TextureOrImage.Handle = sData.Device->CreateTexture(desc);
            handle->TextureOrImage.View = sData.Device->CreateTextureView(RHITextureViewDesc(handle->TextureOrImage.Handle, RHITextureViewType::kShaderRead));

            Uploader::EnqueueTextureUploadRaw(asset.Pixels.data(), asset.Pixels.size(), handle->TextureOrImage.Handle);
            break;
        }
        case AssetType::kImage: {
            ImageData data = Image::LoadImageData(path);

            RHITextureDesc desc = {};
            desc.Width = data.Width;
            desc.Height = data.Height;
            desc.MipLevels = 1;
            desc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
            desc.Usage = RHITextureUsage::kShaderResource;
            
            handle->TextureOrImage.Handle = sData.Device->CreateTexture(desc);
            handle->TextureOrImage.View = sData.Device->CreateTextureView(RHITextureViewDesc(handle->TextureOrImage.Handle, RHITextureViewType::kShaderRead));

            Uploader::EnqueueTextureUploadRaw(data.Pixels.data(), data.Pixels.size(), handle->TextureOrImage.Handle);
            break;
        }
    }

    sData.Assets[path] = handle;
    return handle;
}
 
void AssetManager::Release(Asset::Handle handle)
{
    if (sData.Assets.empty())
        return;
    if (sData.Assets.count(handle->Path) > 0) {
        sData.Assets[handle->Path]->RefCount--;
        if (sData.Assets[handle->Path]->RefCount == 0) {
            SERAPH_INFO("Destroying asset %s", handle->Path.c_str());
            Destroy(sData.Assets[handle->Path]);
        }
    } else {
        SERAPH_WARN("Trying to give back resource {0} that isn't in cache!", handle->Path);
    }
}

void AssetManager::Destroy(Asset::Handle handle)
{
    sData.Assets.erase(handle->Path);
    delete handle;
}
