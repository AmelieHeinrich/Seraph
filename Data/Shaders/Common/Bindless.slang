//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 12:45:44
//

module Bindless;

import BindlessTypes;

#define PUSH_CONSTANTS(Type, Name) [[vk::push_constant]] ConstantBuffer<Type> Name : register(b0)

#ifdef VULKAN

#define DEFINE_CBV_ARRAY(type)                  \
    [[vk::binding(0, 0)]] ConstantBuffer<type> gBindlessCBV_##type[]; \
    public struct BindlessCBV_##type {                                           \
        static type Load(uint index) {                                 \
            return gBindlessCBV_##type[index];                        \
        }                                                             \
    };

#define DEFINE_SRV_ARRAY(type)                        \
    [[vk::binding(0, 0)]] StructuredBuffer<type> gBindlessSRV_##type[];     \
    public struct BindlessSRV_##type {                                                 \
        static StructuredBuffer<type> Load(uint index) {                          \
            return gBindlessSRV_##type[index];                          \
        }                                                               \
    };

#define DEFINE_UAV_ARRAY(type)                          \
    [[vk::binding(0, 0)]] RWStructuredBuffer<type> gBindlessUAV_##type[];     \
    public struct BindlessUAV_##type {                                                   \
        static RWStructuredBuffer<type> Load(uint index) {                          \
            return gBindlessUAV_##type[index];                            \
        }                                                                 \
    };

#else

#define DEFINE_CBV_ARRAY(type)                                                                              \
    public struct BindlessCBV_##type {                                                                                                       \
        static type Load(uint index) {                                                                                        \
            return defaultGetDescriptorFromHandle(DescriptorHandle<ConstantBuffer<type>>(uint2(index, 0)));  \
        }                                                                                                    \
    };

#define DEFINE_SRV_ARRAY(type)                                                                                \
    public struct BindlessCBV_##type {                                                                                                         \
        StructuredBuffer<type> Load(uint index) {                                                                                                     \
            return defaultGetDescriptorFromHandle(DescriptorHandle<StructuredBuffer<type>>(uint2(index, 0))); \
        }                                                                                                   \
    };

#define DEFINE_UAV_ARRAY(type)                                                                                \
    public struct BindlessUAV_##type {                                                                                                         \
        static type Load(uint index) {                                                                                                     \
            return defaultGetDescriptorFromHandle(DescriptorHandle<RWStructuredBuffer<type>>(uint2(index, 0))); \
        }                                                                                                   \
    };

#endif
