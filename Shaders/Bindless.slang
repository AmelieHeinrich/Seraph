//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 15:16:26
//

module Bindless;

#ifdef VULKAN

[[vk::binding(0, 0)]] Texture2D gTexture2DArray[];
[[vk::binding(1, 0)]] SamplerState gSamplerHandles[];
[[vk::binding(2, 0)]] RaytracingAccelerationStructure gRaytracingASArray[];

#else

RaytracingAccelerationStructure gRaytracingASArray[] : register(b1);

#endif

public struct BindlessSampler
{
    int Index;

    public SamplerState Load()
    {
        SamplerState result;
#ifdef VULKAN
        result = gSamplerHandles[Index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<SamplerState>(uint2(Index, 0)));
#endif
        return result;
    }
};

public struct BindlessTexture2D<T:ITexelElement>
{
    int Index;

    public Texture2D Load()
    {
        Texture2D result;
#ifdef VULKAN
        result = gTexture2DArray[Index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<Texture2D>(uint2(Index, 0)));
#endif
        return result;
    }
};
