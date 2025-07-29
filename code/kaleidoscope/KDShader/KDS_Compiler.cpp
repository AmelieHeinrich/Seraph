//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 23:06:21
//

#include "KDS_Compiler.h"

#if defined(KD_WINDOWS)
    #include "DXIL/KDS_DXILCompiler.h"
    #include "SPIRV/KDS_SPIRVCompiler.h"
#endif

namespace KDS
{
    ICompiler* ICompiler::Create(KGPU::ShaderBytecodeType compilerType)
    {
        switch (compilerType) {
#if defined(KD_WINDOWS)
            case KGPU::ShaderBytecodeType::kDXIL: {
                return KC_NEW(DXILCompiler);
            }
            case KGPU::ShaderBytecodeType::kSPIRV: {
                return KC_NEW(SPIRVCompiler);
            }
#endif
            default: {
                return nullptr;
            }
        }
    }
}
