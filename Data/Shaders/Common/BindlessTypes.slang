//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 15:16:26
//

module Bindless;

#ifdef VULKAN

[[vk::binding(0, 0)]] Texture2D<float> gTexture2DArrayFloat[];
[[vk::binding(0, 0)]] Texture2D<float2> gTexture2DArrayFloat2[];
[[vk::binding(0, 0)]] Texture2D<float3> gTexture2DArrayFloat3[];
[[vk::binding(0, 0)]] Texture2D<float4> gTexture2DArrayFloat4[];
[[vk::binding(0, 0)]] RWTexture2D<float> gRWTexture2DArrayFloat[];
[[vk::binding(0, 0)]] RWTexture2D<float2> gRWTexture2DArrayFloat2[];
[[vk::binding(0, 0)]] RWTexture2D<float3> gRWTexture2DArrayFloat3[];
[[vk::binding(0, 0)]] RWTexture2D<float4> gRWTexture2DArrayFloat4[];

[[vk::binding(1, 0)]] SamplerState gSamplerHandles[];
[[vk::binding(2, 0)]] RaytracingAccelerationStructure gRaytracingASArray[];

#define DECLARE_BINDLESS_TEXTURE2D(TypeName, Type)     \
public struct BindlessTexture2D##TypeName              \
{                                                      \
    public static Texture2D<Type> Load(uint index) \
    {                                              \
        Texture2D<Type> result = gTexture2DArray##TypeName[index]; \
        return result;                             \
    }                                                  \
}

#define DECLARE_BINDLESS_RWTEXTURE2D(TypeName, Type)     \
public struct BindlessRWTexture2D##TypeName          \
{                                                  \
    public static RWTexture2D<Type> Load(uint index) \
    {                                              \
        RWTexture2D<Type> result = gRWTexture2DArray##TypeName[index]; \
        return result;                             \
    }                                              \
}

#else

#define DECLARE_BINDLESS_TEXTURE2D(TypeName, Type)                                                       \
public struct BindlessTexture2D##TypeName                                                            \
{                                                                                                      \
    public static Texture2D<Type> Load(uint index)                                                   \
    {                                                                                                  \
        Texture2D<Type> result = defaultGetDescriptorFromHandle(DescriptorHandle<Texture2D<Type>>(uint2(index, 0))); \
        return result;                                                                                 \
    }                                                                                                  \
}

#define DECLARE_BINDLESS_RWTEXTURE2D(TypeName, Type)                                                        \
public struct BindlessRWTexture2D##TypeName                                                           \
{                                                                                                     \
    public static RWTexture2D<Type> Load(uint index)                                                  \
    {                                                                                                 \
        RWTexture2D<Type> result = defaultGetDescriptorFromHandle(DescriptorHandle<RWTexture2D<Type>>(uint2(index, 0)));   \
        return result;                               \
    }                                                \
}

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

public struct BindlessAccelerationStructure
{
    public static RaytracingAccelerationStructure Load(uint index)
    {
        RaytracingAccelerationStructure result;
#ifdef VULKAN
        result = gRaytracingASArray[index];
#else
        result = defaultGetDescriptorFromHandle(DescriptorHandle<RaytracingAccelerationStructure>(uint2(index, 0)));
#endif
        return result;
    }
};

DECLARE_BINDLESS_TEXTURE2D(Float, float);
DECLARE_BINDLESS_TEXTURE2D(Float2, float2);
DECLARE_BINDLESS_TEXTURE2D(Float3, float3);
DECLARE_BINDLESS_TEXTURE2D(Float4, float4);
DECLARE_BINDLESS_RWTEXTURE2D(Float, float);
DECLARE_BINDLESS_RWTEXTURE2D(Float2, float2);
DECLARE_BINDLESS_RWTEXTURE2D(Float3, float3);
DECLARE_BINDLESS_RWTEXTURE2D(Float4, float4);
