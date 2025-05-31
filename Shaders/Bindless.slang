//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 15:16:26
//

module Bindless;

#ifdef VULKAN

[[vk::binding(0, 0)]] __DynamicResource<__DynamicResourceKind.General> gResourceHandles[];
[[vk::binding(0, 1)]] __DynamicResource<__DynamicResourceKind.Sampler> gSamplerHandles[];
[[vk::binding(0, 2)]] RaytracingAccelerationStructure gRaytracingASArray[];

#else

RaytracingAccelerationStructure gRaytracingASArray[] : register(b1);

#endif

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
#ifdef VULKAN
    if (T.kind == __DynamicResourceKind.Sampler)
        return gSamplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
    else if (T.kind == __DynamicResourceKind.General)
        return gResourceHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
    return gResourceHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
#else
    return defaultGetDescriptorFromHandle(handle);
#endif
}

export RaytracingAccelerationStructure GetAS(int index)
{
    return gRaytracingASArray[index];
}
