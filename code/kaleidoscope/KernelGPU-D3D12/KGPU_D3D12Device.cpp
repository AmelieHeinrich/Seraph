//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 19:04:09
//

#include "KGPU_D3D12Device.h"

#include <KernelCore/KC_Assert.h>

#include <unordered_map>

extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

    __declspec(dllexport) extern const uint D3D12SDKVersion = 614;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\.\\";
}

namespace KGPU
{
    D3D12Device::D3D12Device(bool validationLayers)
    {
        IDXGIFactory1* tempFactory;
        HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&tempFactory));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create DXGI factory!");
        tempFactory->QueryInterface(IID_PPV_ARGS(&mFactory));
        tempFactory->Release();

        // Create debug interface.
        if (validationLayers) {
            ID3D12Debug* debug;
            result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
            KD_ASSERT_EQ(SUCCEEDED(result), "Failed to get debug interface!");
            debug->QueryInterface(IID_PPV_ARGS(&mDebug));
            debug->Release();

            mDebug->EnableDebugLayer();
            mDebug->SetEnableGPUBasedValidation(true);
        }

        // Get adapter.
        std::unordered_map<IDXGIAdapter1*, uint64_t> adapterScores;
        for (uint adapterIndex = 0;; adapterIndex++) {
            IDXGIAdapter1* adapter;
            if (FAILED(mFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))))
                break;

            adapterScores[adapter] = CalculateAdapterScore(adapter);
        }

        std::pair<IDXGIAdapter1*, uint64_t> bestAdapter = { nullptr, 0 };
        for (auto& pair : adapterScores) {
            DXGI_ADAPTER_DESC1 desc;
            pair.first->GetDesc1(&desc);

            if (pair.second > bestAdapter.second) {
                bestAdapter = pair;
            }
        }
        mAdapter = bestAdapter.first;

        DXGI_ADAPTER_DESC1 desc;
        mAdapter->GetDesc1(&desc);

        KD_INFO("Choosing D3D12 GPU : %s", UNICODE_TO_MULTIBYTE(desc.Description));

        // Create device.
        result = D3D12CreateDevice(mAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
        KD_ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 device!");

        // Create info queue.
        result = mDevice->QueryInterface(IID_PPV_ARGS(&mInfoQueue));
        if (SUCCEEDED(result)) {
            mInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

            D3D12_MESSAGE_SEVERITY supressSeverities[] = {
                D3D12_MESSAGE_SEVERITY_INFO
            };
            D3D12_MESSAGE_ID supressIDs[] = {
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
                D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
                D3D12_MESSAGE_ID_INCOMPATIBLE_BARRIER_ACCESS
            };

            D3D12_INFO_QUEUE_FILTER filter = {0};
            filter.DenyList.NumSeverities = ARRAYSIZE(supressSeverities);
            filter.DenyList.pSeverityList = supressSeverities;
            filter.DenyList.NumIDs = ARRAYSIZE(supressIDs);
            filter.DenyList.pIDList = supressIDs;

            mInfoQueue->PushStorageFilter(&filter);
        }

        mManager = KC_NEW(D3D12BindlessManager, this);

        CODE_BLOCK("Check for features") {
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 raytracingData = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS7 meshShaderData = {};
            mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &raytracingData, sizeof(raytracingData));
            mDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &meshShaderData, sizeof(meshShaderData));

            if (raytracingData.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1) {
                mSupportsRT = true;
            }
            if (meshShaderData.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1) {
                mSupportsMS = true;
            }
        }

        KD_INFO("Created D3D12 device!");
    }

    D3D12Device::~D3D12Device()
    {
        KC_DELETE(mManager);
        if (mInfoQueue) mInfoQueue->Release();
        if (mDevice) mDevice->Release();
        if (mAdapter) mAdapter->Release();
        if (mDebug) mDebug->Release();
        if (mFactory) mFactory->Release();
    }

    ISurface* D3D12Device::CreateSurface(KOS::IWindow* window, ICommandQueue* graphicsQueue)
    {
        return KC_NEW(D3D12Surface, this, window, static_cast<D3D12CommandQueue*>(graphicsQueue));
    }

    ITexture* D3D12Device::CreateTexture(TextureDesc desc)
    {
        return KC_NEW(D3D12Texture, this, desc);
    }

    ITextureView* D3D12Device::CreateTextureView(TextureViewDesc desc)
    {
        return KC_NEW(D3D12TextureView, this, desc);
    }

    ICommandQueue* D3D12Device::CreateCommandQueue(CommandQueueType type)
    {
        return KC_NEW(D3D12CommandQueue, this, type);
    }

    ISync* D3D12Device::CreateSync(ISurface* surface, ICommandQueue* queue)
    {
    return KC_NEW(D3D12Sync, this, static_cast<D3D12Surface*>(surface), static_cast<D3D12CommandQueue*>(queue));
    }

    IGraphicsPipeline* D3D12Device::CreateGraphicsPipeline(GraphicsPipelineDesc desc)
    {
        return KC_NEW(D3D12GraphicsPipeline, this, desc);
    }

    IBuffer* D3D12Device::CreateBuffer(BufferDesc desc)
    {
        return KC_NEW(D3D12Buffer, this, desc);
    }

    ISampler* D3D12Device::CreateSampler(SamplerDesc desc)
    {
        return KC_NEW(D3D12Sampler, this, desc);
    }

    IComputePipeline* D3D12Device::CreateComputePipeline(ComputePipelineDesc desc)
    {
        return KC_NEW(D3D12ComputePipeline, this, desc);
    }

    IBLAS* D3D12Device::CreateBLAS(BLASDesc desc)
    {
        return KC_NEW(D3D12BLAS, this, desc);
    }

    ITLAS* D3D12Device::CreateTLAS()
    {
        return KC_NEW(D3D12TLAS, this);
    }

    IBufferView* D3D12Device::CreateBufferView(BufferViewDesc desc)
    {
        return KC_NEW(D3D12BufferView, this, desc);
    }

    uint64 D3D12Device::CalculateAdapterScore(IDXGIAdapter1* adapter)
    {
        ID3D12Device* device;
        HRESULT result = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        if (FAILED(result))
            return 0;

        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            return 0;

        uint64_t resultScore = 0;
        resultScore += desc.DedicatedVideoMemory;

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 raytracingData = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS6 VRSData = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 meshShaderData = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS21 workGraphData = {};

        device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &raytracingData, sizeof(raytracingData));
        device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &VRSData, sizeof(VRSData));
        device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &meshShaderData, sizeof(meshShaderData));
        device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &workGraphData, sizeof(workGraphData));

        if (raytracingData.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1) {
            resultScore += 10000;
        }
        if (VRSData.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1) {
            resultScore += 10000;
        }
        if (meshShaderData.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1) {
            resultScore += 10000;
        }
        if (workGraphData.WorkGraphsTier >= D3D12_WORK_GRAPHS_TIER_1_0) {
            resultScore += 10000;
        }
        device->Release();

        return resultScore;
    }
}
