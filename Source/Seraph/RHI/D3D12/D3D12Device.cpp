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

D3D12Device::D3D12Device(bool validationLayers)
{
    SERAPH_INFO("Created D3D12 device!");
}

D3D12Device::~D3D12Device()
{
    
}

IRHISurface* D3D12Device::CreateSurface(Window* window)
{
    return (new D3D12Surface(this, window));
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
