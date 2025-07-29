//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 22:12:13
//

#include "KGPU_D3D12GraphicsPipeline.h"
#include "KGPU_D3D12Device.h"
#include "KGPU_D3D12Texture.h"

#include <Agility/d3dx12/d3dx12.h>

namespace KGPU
{
    D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12Device* device, GraphicsPipelineDesc desc)
    {
        mDesc = desc;
    
        ShaderModule& vertexModule = desc.Modules[ShaderStage::kVertex];
        ShaderModule& fragmentModule = desc.Modules[ShaderStage::kPixel];
    
        D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
        Desc.VS.pShaderBytecode = vertexModule.Data.data();
        Desc.VS.BytecodeLength = vertexModule.Data.size();
        Desc.PS.pShaderBytecode = fragmentModule.Data.data();
        Desc.PS.BytecodeLength = fragmentModule.Data.size();
        for (int RTVIndex = 0; RTVIndex < desc.RenderTargetFormats.size(); RTVIndex++) {
            Desc.BlendState.RenderTarget[RTVIndex].SrcBlend = D3D12_BLEND_ONE;
            Desc.BlendState.RenderTarget[RTVIndex].DestBlend = D3D12_BLEND_ZERO;
            Desc.BlendState.RenderTarget[RTVIndex].BlendOp = D3D12_BLEND_OP_ADD;
            Desc.BlendState.RenderTarget[RTVIndex].SrcBlendAlpha = D3D12_BLEND_ONE;
            Desc.BlendState.RenderTarget[RTVIndex].DestBlendAlpha = D3D12_BLEND_ZERO;
            Desc.BlendState.RenderTarget[RTVIndex].BlendOpAlpha = D3D12_BLEND_OP_ADD;
            Desc.BlendState.RenderTarget[RTVIndex].LogicOp = D3D12_LOGIC_OP_NOOP;
            Desc.BlendState.RenderTarget[RTVIndex].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        
            Desc.RTVFormats[RTVIndex] = D3D12Texture::TranslateToDXGIFormat(desc.RenderTargetFormats[RTVIndex]);
            Desc.NumRenderTargets = desc.RenderTargetFormats.size();
        }
        Desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
        Desc.RasterizerState.FillMode = ToD3DFillMode(desc.FillMode);
        Desc.RasterizerState.CullMode = ToD3DCullMode(desc.CullMode);
        Desc.RasterizerState.DepthClipEnable = false;
        Desc.RasterizerState.FrontCounterClockwise = desc.CounterClockwise;
        Desc.PrimitiveTopologyType = desc.LineTopology ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        if (desc.DepthEnabled) {
            Desc.DepthStencilState.DepthEnable = true;
            if (desc.DepthWrite)
                Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            else
                Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            Desc.DepthStencilState.DepthFunc = ToD3DCompareOp(desc.DepthOperation);
            Desc.DSVFormat = D3D12Texture::TranslateToDXGIFormat(desc.DepthFormat);
            if (desc.DepthClampEnabled) {
                Desc.RasterizerState.DepthBias = 0;
                Desc.RasterizerState.DepthBiasClamp = 0.0f;
                Desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
            }
        }
        if (desc.EnableBlend) {
            for (int i = 0; i < desc.RenderTargetFormats.size(); i++) {
                Desc.BlendState.RenderTarget[i].BlendEnable = TRUE;
                Desc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;
                Desc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
                Desc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
                Desc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
                Desc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
                Desc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
                Desc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
            }
        }
        Desc.SampleDesc.Count = 1;
    
        if (desc.PushConstantSize > 0) {
            CD3DX12_ROOT_PARAMETER1 rootParameters[1] = {};
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
    
        HRESULT result = device->GetDevice()->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&mPipelineState));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 graphics pipeline!");
    
        KD_WHATEVER("Created D3D12 graphics pipeline");
    }
    
    D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
    {
        mPipelineState->Release();
        mRootSignature->Release();    
    }
    
    D3D12_COMPARISON_FUNC D3D12GraphicsPipeline::ToD3DCompareOp(DepthOperation op)
    {
        switch (op)
        {
        case DepthOperation::kGreater:    return D3D12_COMPARISON_FUNC_GREATER;
        case DepthOperation::kEqual:      return D3D12_COMPARISON_FUNC_EQUAL;
        case DepthOperation::kLessEqual:  return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case DepthOperation::kLess:       return D3D12_COMPARISON_FUNC_LESS;
        case DepthOperation::kNone:       return D3D12_COMPARISON_FUNC_ALWAYS;
        default:                             return D3D12_COMPARISON_FUNC_LESS; // Reasonable default
        }
        return D3D12_COMPARISON_FUNC_LESS;
    }
    
    D3D12_CULL_MODE D3D12GraphicsPipeline::ToD3DCullMode(CullMode mode)
    {
        switch (mode)
        {
        case CullMode::kBack:    return D3D12_CULL_MODE_BACK;
        case CullMode::kFront:   return D3D12_CULL_MODE_FRONT;
        case CullMode::kNone:    return D3D12_CULL_MODE_NONE;
        default:                    return D3D12_CULL_MODE_BACK;
        }
        return D3D12_CULL_MODE_BACK;
    }
    
    D3D12_FILL_MODE D3D12GraphicsPipeline::ToD3DFillMode(FillMode mode)
    {
        switch (mode)
        {
        case FillMode::kSolid:      return D3D12_FILL_MODE_SOLID;
        case FillMode::kWireframe:  return D3D12_FILL_MODE_WIREFRAME;
        default:                       return D3D12_FILL_MODE_SOLID;
        }
        return D3D12_FILL_MODE_SOLID;
    }
}
