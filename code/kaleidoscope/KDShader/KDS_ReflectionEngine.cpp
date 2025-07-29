//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 22:01:21
//

#include "KDS_ReflectionEngine.h"

#if defined(KD_WINDOWS)
    #include "DXIL/KDS_DXILReflectionEngine.h"
    #include "SPIRV/KDS_SPIRVReflectionEngine.h"
#endif

namespace KDS
{
    IReflectionEngine* IReflectionEngine::Create(KGPU::ShaderBytecodeType compilerType)
    {
        switch (compilerType) {
#if defined(KD_WINDOWS)
            case KGPU::ShaderBytecodeType::kDXIL: {
                return KC_NEW(DXILReflectionEngine);
            }
            case KGPU::ShaderBytecodeType::kSPIRV: {
                return KC_NEW(SPIRVReflectionEngine);
            }
#endif
            default: {
                return nullptr;
            }
        }
    }
}
