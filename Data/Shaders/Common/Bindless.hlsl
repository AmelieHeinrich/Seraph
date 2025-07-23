//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 12:45:44
//

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

// For normal Texture2D
#define DECLARE_BINDLESS_TEXTURE2D(TypeName, Type)                     \
    Texture2D<Type> BindlessTexture2D##TypeName##_Load(uint index)    \
    {                                                                 \
        Texture2D<Type> result = gTexture2DArray##TypeName[index];    \
        return result;                                                \
    }

// For RWTexture2D
#define DECLARE_BINDLESS_RWTEXTURE2D(TypeName, Type)                  \
    RWTexture2D<Type> BindlessRWTexture2D##TypeName##_Load(uint index)\
    {                                                                 \
        RWTexture2D<Type> result = gRWTexture2DArray##TypeName[index];\
        return result;                                                \
    }

#else

#define DECLARE_BINDLESS_TEXTURE2D(TypeName, Type)                     \
    Texture2D<Type> BindlessTexture2D##TypeName##_Load(uint index)    \
    {                                                                 \
        Texture2D<Type> result = ResourceDescriptorHeap[index];       \
        return result;                                                \
    }

#define DECLARE_BINDLESS_RWTEXTURE2D(TypeName, Type)                  \
    RWTexture2D<Type> BindlessRWTexture2D##TypeName##_Load(uint index)\
    {                                                                 \
        RWTexture2D<Type> result = ResourceDescriptorHeap[index];     \
        return result;                                                \
    }

#endif

SamplerState BindlessSampler_Load(uint index)
{
    SamplerState result;
#ifdef VULKAN
    result = gSamplerHandles[index];
#else
    result = SamplerDescriptorHeap[index];
#endif
    return result;
}

// RaytracingAccelerationStructure loader
RaytracingAccelerationStructure BindlessAccelerationStructure_Load(uint index)
{
    RaytracingAccelerationStructure result;
#ifdef VULKAN
    result = gRaytracingASArray[index];
#else
    result = ResourceDescriptorHeap[index];
#endif
    return result;
}

DECLARE_BINDLESS_TEXTURE2D(Float, float);
DECLARE_BINDLESS_TEXTURE2D(Float2, float2);
DECLARE_BINDLESS_TEXTURE2D(Float3, float3);
DECLARE_BINDLESS_TEXTURE2D(Float4, float4);
DECLARE_BINDLESS_RWTEXTURE2D(Float, float);
DECLARE_BINDLESS_RWTEXTURE2D(Float2, float2);
DECLARE_BINDLESS_RWTEXTURE2D(Float3, float3);
DECLARE_BINDLESS_RWTEXTURE2D(Float4, float4);

static const uint INVALID_DESCRIPTOR = 0x00000000;

#ifdef VULKAN
#define PUSH_CONSTANTS(Type, Name) [[vk::push_constant]] ConstantBuffer<Type> Name : register(b0)
#else
#define PUSH_CONSTANTS(Type, Name) ConstantBuffer<Type> Name : register(b0)
#endif

#ifdef VULKAN

#define DEFINE_CBV_ARRAY(type)                                          \
    [[vk::binding(0, 0)]] ConstantBuffer<type> gBindlessCBV_##type[]; \
    ConstantBuffer<type> BindlessCBV_##type##_Load(uint index) {       \
        return gBindlessCBV_##type[index];                             \
    }

#define DEFINE_SRV_ARRAY(type)                                              \
    [[vk::binding(0, 0)]] StructuredBuffer<type> gBindlessSRV_##type[];   \
    StructuredBuffer<type> BindlessSRV_##type##_Load(uint index) {         \
        return gBindlessSRV_##type[index];                                 \
    }

#define DEFINE_UAV_ARRAY(type)                                                \
    [[vk::binding(0, 0)]] RWStructuredBuffer<type> gBindlessUAV_##type[];   \
    RWStructuredBuffer<type> BindlessUAV_##type##_Load(uint index) {         \
        return gBindlessUAV_##type[index];                                   \
    }

#else

#define DEFINE_CBV_ARRAY(type)                                       \
    ConstantBuffer<type> BindlessCBV_##type##_Load(uint index) {    \
        return ResourceDescriptorHeap[index];                        \
    }

#define DEFINE_SRV_ARRAY(type)                                             \
    StructuredBuffer<type> BindlessSRV_##type##_Load(uint index) {        \
        return ResourceDescriptorHeap[index];                              \
    }

#define DEFINE_UAV_ARRAY(type)                                                  \
    RWStructuredBuffer<type> BindlessUAV_##type##_Load(uint index) {          \
        return ResourceDescriptorHeap[index];                                   \
    }

#endif
