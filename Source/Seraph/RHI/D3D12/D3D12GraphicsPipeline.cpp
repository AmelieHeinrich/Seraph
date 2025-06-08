//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 13:58:42
//

#include "D3D12GraphicsPipeline.h"
#include "D3D12Texture.h"
#include "D3D12Device.h"

#include <DXC/dxcapi.h>
#include <d3d12shader.h>
#include <Agility/d3dx12/d3dx12.h>

ID3D12ShaderReflection* ReflectShader(ShaderModule shader)
{
    ID3D12ShaderReflection* pReflection = nullptr;
    
    IDxcUtils* pUtils = nullptr;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
    
    DxcBuffer ShaderBuffer = {};
    ShaderBuffer.Ptr = shader.Bytecode.data();
    ShaderBuffer.Size = shader.Bytecode.size();
    
    ASSERT_EQ(SUCCEEDED(pUtils->CreateReflection(&ShaderBuffer, IID_PPV_ARGS(&pReflection))), "Failed to get shader reflection!");
    pUtils->Release();
    return pReflection;
}

D3D12GraphicsPipeline::D3D12GraphicsPipeline(D3D12Device* device, RHIGraphicsPipelineDesc desc)
{
    ShaderModule vertexModule = desc.Bytecode[ShaderStage::kVertex];
    ShaderModule fragmentModule = desc.Bytecode[ShaderStage::kFragment];

    ID3D12ShaderReflection* pVertexReflection = ReflectShader(vertexModule);
    D3D12_SHADER_DESC vertexDesc;
    pVertexReflection->GetDesc(&vertexDesc);

    Array<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;
    Array<std::string> InputElementSemanticNames;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
    Desc.VS.pShaderBytecode = vertexModule.Bytecode.data();
    Desc.VS.BytecodeLength = vertexModule.Bytecode.size();
    Desc.PS.pShaderBytecode = fragmentModule.Bytecode.data();
    Desc.PS.BytecodeLength = fragmentModule.Bytecode.size();
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
    Desc.SampleDesc.Count = 1;

    InputElementSemanticNames.reserve(vertexDesc.InputParameters);
    InputElementDescs.reserve(vertexDesc.InputParameters);

    for (int ParameterIndex = 0; ParameterIndex < vertexDesc.InputParameters; ParameterIndex++)
    {
        D3D12_SIGNATURE_PARAMETER_DESC ParameterDesc = {};
        pVertexReflection->GetInputParameterDesc(ParameterIndex, &ParameterDesc);

        InputElementSemanticNames.push_back(ParameterDesc.SemanticName);

        D3D12_INPUT_ELEMENT_DESC InputElement = {};
        InputElement.SemanticName = InputElementSemanticNames.back().c_str();
        InputElement.SemanticIndex = ParameterDesc.SemanticIndex;
        InputElement.InputSlot = 0;
        InputElement.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElement.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        InputElement.InstanceDataStepRate = 0;

        if (ParameterDesc.Mask == 1)
        {
            if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) InputElement.Format = DXGI_FORMAT_R32_UINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) InputElement.Format = DXGI_FORMAT_R32_SINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) InputElement.Format = DXGI_FORMAT_R32_FLOAT;
        }
        else if (ParameterDesc.Mask <= 3)
        {
            if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) InputElement.Format = DXGI_FORMAT_R32G32_UINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) InputElement.Format = DXGI_FORMAT_R32G32_SINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) InputElement.Format = DXGI_FORMAT_R32G32_FLOAT;
        }
        else if (ParameterDesc.Mask <= 7)
        {
            if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) InputElement.Format = DXGI_FORMAT_R32G32B32_UINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) InputElement.Format = DXGI_FORMAT_R32G32B32_SINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) InputElement.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        }
        else if (ParameterDesc.Mask <= 15)
        {
            if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) InputElement.Format = DXGI_FORMAT_R32G32B32A32_UINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) InputElement.Format = DXGI_FORMAT_R32G32B32A32_SINT;
            else if (ParameterDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) InputElement.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }

        InputElementDescs.push_back(InputElement);
    }
    if (desc.ReflectInputLayout) {
        Desc.InputLayout.pInputElementDescs = InputElementDescs.data();
        Desc.InputLayout.NumElements = static_cast<uint>(InputElementDescs.size());
    }

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
        ASSERT_EQ(SUCCEEDED(hr), "Failed to serialize D3D12 root siganture!");

        hr = device->GetDevice()->CreateRootSignature(
            0,
            signatureBlob->GetBufferPointer(),
            signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&mRootSignature)
        );
        ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

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
        ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

        hr = device->GetDevice()->CreateRootSignature(
            0,
            signatureBlob->GetBufferPointer(),
            signatureBlob->GetBufferSize(),
            IID_PPV_ARGS(&mRootSignature)
        );
        ASSERT_EQ(SUCCEEDED(hr), "Failed to create D3D12 root siganture!");

        if (signatureBlob) signatureBlob->Release();
        if (errorBlob) errorBlob->Release();
    }

    Desc.pRootSignature = mRootSignature;

    HRESULT result = device->GetDevice()->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&mPipelineState));
    ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 graphics pipeline!");

    SERAPH_WHATEVER("Created D3D12 graphics pipeline");
}

D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
{
    mPipelineState->Release();
    mRootSignature->Release();    
}

D3D12_COMPARISON_FUNC D3D12GraphicsPipeline::ToD3DCompareOp(RHIDepthOperation op)
{
    switch (op)
    {
    case RHIDepthOperation::kGreater:    return D3D12_COMPARISON_FUNC_GREATER;
    case RHIDepthOperation::kEqual:      return D3D12_COMPARISON_FUNC_EQUAL;
    case RHIDepthOperation::kLessEqual:  return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case RHIDepthOperation::kLess:       return D3D12_COMPARISON_FUNC_LESS;
    case RHIDepthOperation::kNone:       return D3D12_COMPARISON_FUNC_ALWAYS;
    default:                             return D3D12_COMPARISON_FUNC_LESS; // Reasonable default
    }
    return D3D12_COMPARISON_FUNC_LESS;
}

D3D12_CULL_MODE D3D12GraphicsPipeline::ToD3DCullMode(RHICullMode mode)
{
    switch (mode)
    {
    case RHICullMode::kBack:    return D3D12_CULL_MODE_BACK;
    case RHICullMode::kFront:   return D3D12_CULL_MODE_FRONT;
    case RHICullMode::kNone:    return D3D12_CULL_MODE_NONE;
    default:                    return D3D12_CULL_MODE_BACK;
    }
    return D3D12_CULL_MODE_BACK;
}

D3D12_FILL_MODE D3D12GraphicsPipeline::ToD3DFillMode(RHIFillMode mode)
{
    switch (mode)
    {
    case RHIFillMode::kSolid:      return D3D12_FILL_MODE_SOLID;
    case RHIFillMode::kWireframe:  return D3D12_FILL_MODE_WIREFRAME;
    default:                       return D3D12_FILL_MODE_SOLID;
    }
    return D3D12_FILL_MODE_SOLID;
}
