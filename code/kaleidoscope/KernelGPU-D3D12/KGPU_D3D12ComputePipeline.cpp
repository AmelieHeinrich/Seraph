//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:44:57
//

#include "KGPU_D3D12ComputePipeline.h"
#include "KGPU_D3D12Device.h"

#include <Agility/d3dx12/d3dx12.h>

namespace KGPU
{
    D3D12ComputePipeline::D3D12ComputePipeline(D3D12Device* device, ComputePipelineDesc desc)
    {
        KD_ASSERT_EQ(desc.ComputeBytecode.Bytecode == KGPU::ShaderBytecodeType::kDXIL, "Bytecode must be DXIL for D3D12 pipeline!");

        D3D12_COMPUTE_PIPELINE_STATE_DESC Desc = {};
        Desc.CS.pShaderBytecode = desc.ComputeBytecode.Data.data();
        Desc.CS.BytecodeLength = desc.ComputeBytecode.Data.size();

        if (desc.PushConstantSize > 0) {
            CD3DX12_ROOT_PARAMETER1 rootParameters[1];
            rootParameters[0].InitAsConstants(desc.PushConstantSize / 4, 0, D3D12_SHADER_VISIBILITY_ALL);

            D3D12_ROOT_SIGNATURE_FLAGS rootSigFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                                                    | D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED
                                                    | D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init_1_1(1, rootParameters, 0, nullptr, rootSigFlags);

            ID3DBlob* signatureBlob;
            ID3DBlob* errorBlob;
            HRESULT hr = D3DX12SerializeVersionedRootSignature(
                &rootSignatureDesc,
                D3D_ROOT_SIGNATURE_VERSION_1_1,
                &signatureBlob,
                &errorBlob
            );
            KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to serialize D3D12 root siganture!");

            hr = device->GetDevice()->CreateRootSignature(
                0,
                signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&mRootSignature)
            );
            KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

            if (signatureBlob) signatureBlob->Release();
            if (errorBlob) errorBlob->Release();
        } else {
            D3D12_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
            RootSignatureDesc.NumParameters = 0;
            RootSignatureDesc.pParameters = nullptr;
            RootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            ID3DBlob* signatureBlob;
            ID3DBlob* errorBlob;
            HRESULT hr = D3D12SerializeRootSignature(&RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, &errorBlob);
            KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

            hr = device->GetDevice()->CreateRootSignature(
                0,
                signatureBlob->GetBufferPointer(),
                signatureBlob->GetBufferSize(),
                IID_PPV_ARGS(&mRootSignature)
            );
            KD_ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

            if (signatureBlob) signatureBlob->Release();
            if (errorBlob) errorBlob->Release();
        }
        Desc.pRootSignature = mRootSignature;

        HRESULT result = device->GetDevice()->CreateComputePipelineState(&Desc, IID_PPV_ARGS(&mPipelineState));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 compute pipeline state!");

        KD_WHATEVER("Created D3D12 compute pipeline state!");
    }

    D3D12ComputePipeline::~D3D12ComputePipeline()
    {
        if (mRootSignature) mRootSignature->Release();
        if (mPipelineState) mPipelineState->Release();
    }
}
