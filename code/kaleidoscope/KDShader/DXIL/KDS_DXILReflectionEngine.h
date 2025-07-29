//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 21:57:58
//

#pragma once

#include "KDS_ReflectionEngine.h"

#include <KernelOS/KOS_DynLib.h>

namespace KDS
{
    class DXILReflectionEngine : public IReflectionEngine
    {
    public:
        DXILReflectionEngine();
        ~DXILReflectionEngine() override;

        ReflectionResult Reflect(const KGPU::ShaderModule& module) override;
    };
}
