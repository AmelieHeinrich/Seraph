//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 22:32:39
//

#pragma once

#include <KernelCore/KC_Array.h>
#include <KernelCore/KC_HashMap.h>
#include <KernelGPU/KGPU_ShaderModule.h>

#include "KDS_Parser.h"
#include "KDS_Compiler.h"
#include "KDS_ReflectionEngine.h"

namespace KDS
{
    class ShaderFile
    {
    public:
        ShaderFile(ICompiler* compiler, IReflectionEngine* engine, const KC::String& path);
        ~ShaderFile();

        KGPU::ShaderModule Get(KGPU::ShaderStage stage);

        ReflectionResult GetReflection() const { return mReflection; }
    private:
        ReflectionResult mReflection;
        KC::HashMap<KGPU::ShaderStage, KGPU::ShaderModule> mModules;
    };
}
