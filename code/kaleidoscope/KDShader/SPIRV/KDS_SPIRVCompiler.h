//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 23:06:30
//

#pragma once

#include "KDS_Compiler.h"

#include <KernelOS/KOS_DynLib.h>

namespace KDS
{
    class SPIRVCompiler : public ICompiler
    {
    public:
        SPIRVCompiler();
        ~SPIRVCompiler() override;

        KGPU::ShaderModule Compile(const KC::String& source, const KC::String& entry, KGPU::ShaderStage stage) override;
    };
}
