//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 21:59:33
//

#include "KDS_DXILReflectionEngine.h"
#include "KDS_Manager.h"

#include <Windows.h>
#include <DXC/dxcapi.h>
#include <d3d12shader.h>

namespace KDS
{
    typedef HRESULT (*PFN_DxcCreateInstance)(REFCLSID rclsid, REFIID riid, LPVOID *ppv);

    DXILReflectionEngine::DXILReflectionEngine()
    {
       
    }

    DXILReflectionEngine::~DXILReflectionEngine()
    {
        
    }

    ReflectionResult DXILReflectionEngine::Reflect(const KGPU::ShaderModule& module)
    {
        PFN_DxcCreateInstance dxcCreate = (PFN_DxcCreateInstance)Manager::GetDynLib()->GetFunction("DxcCreateInstance");
        ID3D12ShaderReflection* pReflection = nullptr;
    
        IDxcUtils* pUtils = nullptr;
        dxcCreate(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
        
        DxcBuffer ShaderBuffer = {};
        ShaderBuffer.Ptr = module.Data.data();
        ShaderBuffer.Size = module.Data.size();

        KD_ASSERT_EQ(ShaderBuffer.Size > 0, "Shader buffer size must be above 0!");
        KD_ASSERT_EQ(SUCCEEDED(pUtils->CreateReflection(&ShaderBuffer, IID_PPV_ARGS(&pReflection))), "Failed to get shader reflection!");
        pUtils->Release();

        ReflectionResult result = {};

        auto pushConstantBuffer = pReflection->GetConstantBufferByIndex(0);
        if (pushConstantBuffer) {
            D3D12_SHADER_BUFFER_DESC bufferDesc = {};
            pushConstantBuffer->GetDesc(&bufferDesc);
            result.PushConstantSize = bufferDesc.Size;
        }
        if (module.Stage == KGPU::ShaderStage::kCompute) {
            pReflection->GetThreadGroupSize(&result.ThreadsPerGroup.x, &result.ThreadsPerGroup.y, &result.ThreadsPerGroup.z);
        }

        return result;
    }
}
