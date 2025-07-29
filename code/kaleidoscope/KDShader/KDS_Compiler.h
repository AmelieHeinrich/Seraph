//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 23:02:52
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelGPU/KGPU_ShaderModule.h>

namespace KDS
{
    class ICompiler
    {
    public:
        virtual ~ICompiler() = default;

        static ICompiler* Create(KGPU::ShaderBytecodeType compilerType);

        virtual KGPU::ShaderModule Compile(const KC::String& source, const KC::String& entry, KGPU::ShaderStage stage) = 0;
    };
}
