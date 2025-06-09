//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-09 12:16:22
//

#pragma once

#include "Model.h"
#include "Image.h"
#include "Texture.h"

#include <RHI/Device.h>

enum class AssetType
{
    kModel,
    kTexture,
    kImage
};

struct Asset
{
    String Path;
    AssetType Type;

    Model* Model;
    struct {
        IRHITexture* Handle;
        IRHITextureView* View;
    } TextureOrImage;

    int RefCount;

    using Handle = Asset*;

    ~Asset();
};

class AssetManager
{
public:
    static void Initialize(IRHIDevice* device);
    static void Shutdown();

    static void Update();

    static Asset::Handle Get(const String& path, AssetType type);
    static void Release(Asset::Handle handle);
    static void Destroy(Asset::Handle handle);

private:
    static struct Data {
        IRHIDevice* Device;
        UnorderedMap<String, Asset::Handle> Assets;
    } sData;
};
