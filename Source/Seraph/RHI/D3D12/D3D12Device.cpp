//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:33:54
//

#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12Surface.h"
#include "D3D12Texture.h"
#include "D3D12TextureView.h"
#include "D3D12F2FSync.h"
#include "D3D12GraphicsPipeline.h"
#include "D3D12Buffer.h"
#include "D3D12Sampler.h"
#include "D3D12ComputePipeline.h"
#include "D3D12MeshPipeline.h"
#include "D3D12BLAS.h"
#include "D3D12TLAS.h"
#include "D3D12BufferView.h"
#include "D3D12ImGuiContext.h"

#include <Core/String.h>

extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

    __declspec(dllexport) extern const uint D3D12SDKVersion = 614;
    __declspec(dllexport) extern const char* D3D12SDKPath = ".\\.\\";
}

D3D12Device::D3D12Device(bool validationLayers)
{
    IDXGIFactory1* tempFactory;
    HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&tempFactory));
    ASSERT_EQ(SUCCEEDED(result), "Failed to create DXGI factory!");
    tempFactory->QueryInterface(IID_PPV_ARGS(&mFactory));
    tempFactory->Release();

    // Create debug interface.
    if (validationLayers) {
        ID3D12Debug* debug;
        result = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
        ASSERT_EQ(SUCCEEDED(result), "Failed to get debug interface!");
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

    SERAPH_INFO("Selecting GPU %s", UNICODE_TO_MULTIBYTE(desc.Description));

    // Create device.
    result = D3D12CreateDevice(mAdapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
    ASSERT_EQ(SUCCEEDED(result), "Failed to create D3D12 device!");

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

    mBindlessManager = new D3D12BindlessManager(this);

    SERAPH_INFO("Created D3D12 device!");
}

D3D12Device::~D3D12Device()
{
    if (mInfoQueue) mInfoQueue->Release();
    if (mAllocator) mAllocator->Release();
    delete mBindlessManager;
    if (mDevice) mDevice->Release();
    if (mAdapter) mAdapter->Release();
    if (mDebug) mDebug->Release();
    if (mFactory) mFactory->Release();
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

IRHISurface* D3D12Device::CreateSurface(Window* window, IRHICommandQueue* graphicsQueue)
{
    return (new D3D12Surface(this, window, static_cast<D3D12CommandQueue*>(graphicsQueue)));
}

IRHITexture* D3D12Device::CreateTexture(RHITextureDesc desc)
{
    return (new D3D12Texture(this, desc));
}

IRHITextureView* D3D12Device::CreateTextureView(RHITextureViewDesc desc)
{
    return (new D3D12TextureView(this, desc));
}

IRHICommandQueue* D3D12Device::CreateCommandQueue(RHICommandQueueType type)
{
    return (new D3D12CommandQueue(this, type));
}

IRHIF2FSync* D3D12Device::CreateF2FSync(IRHISurface* surface, IRHICommandQueue* queue)
{
    return (new D3D12F2FSync(this, static_cast<D3D12Surface*>(surface), static_cast<D3D12CommandQueue*>(queue)));
}

IRHIGraphicsPipeline* D3D12Device::CreateGraphicsPipeline(RHIGraphicsPipelineDesc desc)
{
    return (new D3D12GraphicsPipeline(this, desc));
}

IRHIBuffer* D3D12Device::CreateBuffer(RHIBufferDesc desc)
{
    return (new D3D12Buffer(this, desc));
}

IRHISampler* D3D12Device::CreateSampler(RHISamplerDesc desc)
{
    return (new D3D12Sampler(this, desc));
}

IRHIComputePipeline* D3D12Device::CreateComputePipeline(RHIComputePipelineDesc desc)
{
    return (new D3D12ComputePipeline(this, desc));
}

IRHIMeshPipeline* D3D12Device::CreateMeshPipeline(RHIMeshPipelineDesc desc)
{
    return (new D3D12MeshPipeline(this, desc));
}

IRHIBLAS* D3D12Device::CreateBLAS(RHIBLASDesc desc)
{
    return (new D3D12BLAS(this, desc));
}

IRHITLAS* D3D12Device::CreateTLAS()
{
    return (new D3D12TLAS(this));
}

IRHIBufferView* D3D12Device::CreateBufferView(RHIBufferViewDesc desc)
{
    return (new D3D12BufferView(this, desc));
}

IRHIImGuiContext* D3D12Device::CreateImGuiContext(IRHICommandQueue* mainQueue, Window* window)
{
    return (new D3D12ImGuiContext(this, static_cast<D3D12CommandQueue*>(mainQueue), window));
}
