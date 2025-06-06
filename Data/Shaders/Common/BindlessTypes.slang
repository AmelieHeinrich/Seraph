//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 15:16:26
//

module Bindless;

#ifdef VULKAN

[[vk::binding(0, 0)]] Texture2D gTexture2DArray[];
[[vk::binding(0, 0)]] RWTexture2D gRWTexture2DArray[];
[[vk::binding(1, 0)]] SamplerState gSamplerHandles[];
[[vk::binding(2, 0)]] RaytracingAccelerationStructure gRaytracingASArray[];

#endif

public struct BindlessSampler
{
    public static SamplerState Load(uint index)
    {
        SamplerState result;
#ifdef VULKAN
        result = gSamplerHandles[index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<SamplerState>(uint2(index, 0)));
#endif
        return result;
    }
};

public struct BindlessTexture2D<T:ITexelElement>
{
    public static Texture2D Load(uint index)
    {
        Texture2D result;
#ifdef VULKAN
        result = gTexture2DArray[index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<Texture2D>(uint2(index, 0)));
#endif
        return result;
    }
};

public struct BindlessRWTexture2D<T : ITexelElement>
{
    public static RWTexture2D Load(uint index)
    {
        RWTexture2D result;
#ifdef VULKAN
        result = gRWTexture2DArray[index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<RWTexture2D>(uint2(index, 0)));
#endif
        return result;
    }
};
