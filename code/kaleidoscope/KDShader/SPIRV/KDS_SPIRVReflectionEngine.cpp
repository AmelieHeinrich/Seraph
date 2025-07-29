//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 21:59:33
//

#include "KDS_SPIRVReflectionEngine.h"
#include "KDS_Manager.h"

#include <Windows.h>
#include <spirv_reflect.h>

namespace KDS
{
    SPIRVReflectionEngine::SPIRVReflectionEngine()
    {
       
    }

    SPIRVReflectionEngine::~SPIRVReflectionEngine()
    {
        
    }

    ReflectionResult SPIRVReflectionEngine::Reflect(const KGPU::ShaderModule& module)
    {
        ReflectionResult result;

        SpvReflectShaderModule spvModule;
        SpvReflectResult err = spvReflectCreateShaderModule(module.Data.size(), module.Data.data(), &spvModule);
        KD_ASSERT_EQ(err == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect shader module!");

        auto block = spvReflectGetPushConstantBlock(&spvModule, 0, &err);

        if (block) {
            result.PushConstantSize = block->size;
        } else {
            KD_WHATEVER("Didn't find push constant block in reflection data!");
            result.PushConstantSize = 0;
        }
        result.ThreadsPerGroup = KGPU::uint3(
            spvModule.entry_points[0].local_size.x,
            spvModule.entry_points[0].local_size.y,
            spvModule.entry_points[0].local_size.z
        );

        spvReflectDestroyShaderModule(&spvModule);
        return result;
    }
}
