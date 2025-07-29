//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 21:45:05
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelGPU/KGPU_ShaderCompat.h>
#include <KernelGPU/KGPU_ShaderModule.h>

namespace KDS
{
    struct ReflectionResult
    {
        uint PushConstantSize = 0;
        KGPU::uint3 ThreadsPerGroup = {};
    };

    class IReflectionEngine
    {
    public:
        virtual ~IReflectionEngine() = default;

        static IReflectionEngine* Create(KGPU::ShaderBytecodeType engineType);

        virtual ReflectionResult Reflect(const KGPU::ShaderModule& module) = 0;
    };
}
