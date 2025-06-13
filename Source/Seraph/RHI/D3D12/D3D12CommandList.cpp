//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:15:07
//

#include "D3D12CommandList.h"
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12TextureView.h"
#include "D3D12Texture.h"
#include "D3D12Buffer.h"
#include "D3D12GraphicsPipeline.h"
#include "D3D12ComputePipeline.h"
#include "D3D12MeshPipeline.h"
#include "D3D12BLAS.h"
#include "D3D12TLAS.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_dx12.h>
#include <ImGui/imgui_impl_sdl3.h>

#include <PIX/pix3.h>

D3D12CommandList::D3D12CommandList(D3D12Device* device, D3D12CommandQueue* queue, bool singleTime)
    : mSingleTime(singleTime), mParentDevice(device)
{
    HRESULT result = device->GetDevice()->CreateCommandAllocator(D3D12CommandQueue::TranslateToD3D12List(queue->GetType()), IID_PPV_ARGS(&mAllocator));
    ASSERT_EQ(SUCCEEDED(result), "Failed to create command allocator!");

    result = device->GetDevice()->CreateCommandList(0, D3D12CommandQueue::TranslateToD3D12List(queue->GetType()), mAllocator, nullptr, IID_PPV_ARGS(&mList));
    ASSERT_EQ(SUCCEEDED(result), "Failed to create command list!");

    if (!singleTime) {
        mList->Close();
    }

    SERAPH_WHATEVER("Created D3D12 command buffer!");
}

D3D12CommandList::~D3D12CommandList()
{
    if (mAllocator) mAllocator->Release();
    if (mList) mList->Release();
}

void D3D12CommandList::Reset()
{
    mAllocator->Reset();
    mList->Reset(mAllocator, nullptr);
}

void D3D12CommandList::Begin()
{
    ID3D12DescriptorHeap* heaps[] = {
        mParentDevice->GetBindlessManager()->GetResourceHeap(),
        mParentDevice->GetBindlessManager()->GetSamplerHeap()
    };
    mList->SetDescriptorHeaps(2, heaps);
}

void D3D12CommandList::End()
{
    mList->Close();
}

void D3D12CommandList::BeginRendering(const RHIRenderBegin& begin)
{
    Array<D3D12_CPU_DESCRIPTOR_HANDLE> cpus;
    for (auto& target : begin.RenderTargets) {
        D3D12TextureView* view = static_cast<D3D12TextureView*>(target.View);
        cpus.push_back(view->GetAlloc().CPU);

        float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        if (target.Clear) mList->ClearRenderTargetView(view->GetAlloc().CPU, color, 0, nullptr);
    }
    D3D12_CPU_DESCRIPTOR_HANDLE depth_cpu = {};
    if (begin.DepthTarget.View) {
        D3D12TextureView* view = static_cast<D3D12TextureView*>(begin.DepthTarget.View);
        depth_cpu = view->GetAlloc().CPU;
        if (begin.DepthTarget.Clear) mList->ClearDepthStencilView(depth_cpu, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    mList->OMSetRenderTargets(cpus.size(), cpus.data(), false, begin.DepthTarget.View ? &depth_cpu : nullptr);
}

void D3D12CommandList::EndRendering()
{
    // Nothing
}

void D3D12CommandList::Barrier(const RHITextureBarrier& barrier)
{
    D3D12_TEXTURE_BARRIER texBarrier = {};
    texBarrier.pResource = static_cast<D3D12Texture*>(barrier.Texture)->GetResource();
    texBarrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
    texBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
    texBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);
    texBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
    texBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
    texBarrier.LayoutBefore = ToD3D12BarrierLayout(barrier.Texture->GetLayout());
    texBarrier.LayoutAfter = ToD3D12BarrierLayout(barrier.NewLayout);
    texBarrier.Subresources.IndexOrFirstMipLevel = barrier.BaseMipLevel;
    texBarrier.Subresources.NumMipLevels = barrier.LevelCount;
    texBarrier.Subresources.FirstArraySlice = barrier.ArrayLayer;
    texBarrier.Subresources.NumArraySlices = barrier.LayerCount;
    texBarrier.Subresources.FirstPlane = 0;
    texBarrier.Subresources.NumPlanes = 1;
    barrier.Texture->SetLayout(barrier.NewLayout);

    D3D12_BARRIER_GROUP group = {};
    group.Type = D3D12_BARRIER_TYPE_TEXTURE;
    group.NumBarriers = 1;
    group.pTextureBarriers = &texBarrier;

    mList->Barrier(1, &group);
}


void D3D12CommandList::Barrier(const RHIBufferBarrier& barrier)
{
    D3D12_BUFFER_BARRIER bufBarrier = {};
    bufBarrier.pResource = static_cast<D3D12Buffer*>(barrier.Buffer)->GetResource();
    bufBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
    bufBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
    bufBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
    bufBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);
    bufBarrier.Offset = 0;
    bufBarrier.Size = barrier.Buffer->GetDesc().Size;

    D3D12_BARRIER_GROUP group = {};
    group.Type = D3D12_BARRIER_TYPE_BUFFER;
    group.NumBarriers = 1;
    group.pBufferBarriers = &bufBarrier;

    mList->Barrier(1, &group);
}

void D3D12CommandList::Barrier(const RHIMemoryBarrier& barrier)
{
    D3D12_GLOBAL_BARRIER globalBarrier = {};
    globalBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
    globalBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
    globalBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
    globalBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);

    D3D12_BARRIER_GROUP group = {};
    group.Type = D3D12_BARRIER_TYPE_GLOBAL;
    group.NumBarriers = 1;
    group.pGlobalBarriers = &globalBarrier;

    mList->Barrier(1, &group);
}

void D3D12CommandList::BarrierGroup(const RHIBarrierGroup& barrierGroup)
{
    Array<D3D12_TEXTURE_BARRIER> textureBarriers;
    Array<D3D12_BUFFER_BARRIER> bufferBarriers;
    Array<D3D12_GLOBAL_BARRIER> globalBarriers;
    Array<D3D12_BARRIER_GROUP> barriers;

    for (const RHITextureBarrier& barrier : barrierGroup.TextureBarriers) {
        D3D12_TEXTURE_BARRIER texBarrier = {};
        texBarrier.pResource = static_cast<D3D12Texture*>(barrier.Texture)->GetResource();
        texBarrier.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE;
        texBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
        texBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);
        texBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
        texBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
        texBarrier.LayoutBefore = ToD3D12BarrierLayout(barrier.Texture->GetLayout());
        texBarrier.LayoutAfter = ToD3D12BarrierLayout(barrier.NewLayout);
        texBarrier.Subresources.IndexOrFirstMipLevel = barrier.BaseMipLevel;
        texBarrier.Subresources.NumMipLevels = barrier.LevelCount;
        texBarrier.Subresources.FirstArraySlice = barrier.ArrayLayer;
        texBarrier.Subresources.NumArraySlices = barrier.LayerCount;
        texBarrier.Subresources.FirstPlane = 0;
        texBarrier.Subresources.NumPlanes = 1;

        textureBarriers.push_back(texBarrier);
    }
    for (const RHIBufferBarrier& barrier : barrierGroup.BufferBarriers) {
        D3D12_BUFFER_BARRIER bufBarrier = {};
        bufBarrier.pResource = static_cast<D3D12Buffer*>(barrier.Buffer)->GetResource();
        bufBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
        bufBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
        bufBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
        bufBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);
        bufBarrier.Offset = 0;
        bufBarrier.Size = barrier.Buffer->GetDesc().Size;

        bufferBarriers.push_back(bufBarrier);
    }
    for (const RHIMemoryBarrier& barrier : barrierGroup.MemoryBarriers) {
        D3D12_GLOBAL_BARRIER globalBarrier = {};
        globalBarrier.AccessBefore = ToD3D12BarrierAccess(barrier.SourceAccess);
        globalBarrier.AccessAfter = ToD3D12BarrierAccess(barrier.DestAccess);
        globalBarrier.SyncBefore = ToD3D12BarrierSync(barrier.SourceStage);
        globalBarrier.SyncAfter = ToD3D12BarrierSync(barrier.DestStage);

        globalBarriers.push_back(globalBarrier);
    }

    D3D12_BARRIER_GROUP texBarriers = {};
    texBarriers.Type = D3D12_BARRIER_TYPE_TEXTURE;
    texBarriers.NumBarriers = textureBarriers.size();
    texBarriers.pTextureBarriers = textureBarriers.data();
    if (texBarriers.NumBarriers > 0) barriers.push_back(texBarriers);

    D3D12_BARRIER_GROUP bufBarriers = {};
    bufBarriers.Type = D3D12_BARRIER_TYPE_BUFFER;
    bufBarriers.NumBarriers = bufferBarriers.size();
    bufBarriers.pBufferBarriers = bufferBarriers.data();
    if (bufBarriers.NumBarriers > 0) barriers.push_back(bufBarriers);

    D3D12_BARRIER_GROUP globBarriers = {};
    globBarriers.Type = D3D12_BARRIER_TYPE_GLOBAL;
    globBarriers.NumBarriers = globalBarriers.size();
    globBarriers.pGlobalBarriers = globalBarriers.data();
    if (globBarriers.NumBarriers > 0) barriers.push_back(globBarriers);  

    mList->Barrier(barriers.size(), barriers.data());

    for (const RHITextureBarrier& barrier : barrierGroup.TextureBarriers) {
        barrier.Texture->SetLayout(barrier.NewLayout);
    }
}

void D3D12CommandList::ClearColor(IRHITextureView* view, float r, float g, float b)
{
    float color[] = { r, g, b, 1.0f };
    mList->ClearRenderTargetView(static_cast<D3D12TextureView*>(view)->GetAlloc().CPU, color, 0, nullptr);
}

void D3D12CommandList::SetGraphicsPipeline(IRHIGraphicsPipeline* pipeline)
{
    D3D12GraphicsPipeline* d3dPipeline = static_cast<D3D12GraphicsPipeline*>(pipeline);

    mList->SetPipelineState(d3dPipeline->GetPipelineState());
    mList->SetGraphicsRootSignature(d3dPipeline->GetRootSignature());

    if (pipeline->GetDesc().LineTopology) {
        mList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    } else {
        mList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
}

void D3D12CommandList::SetViewport(float width, float height, float x, float y)
{
    D3D12_VIEWPORT viewport = {};
    viewport.Width = width;
    viewport.Height = height;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT rect = {};
    rect.right = width;
    rect.bottom = height;
    rect.top = x;
    rect.left = y;

    mList->RSSetScissorRects(1, &rect);
    mList->RSSetViewports(1, &viewport);
}

void D3D12CommandList::SetVertexBuffer(IRHIBuffer* buffer)
{
    D3D12_VERTEX_BUFFER_VIEW view = {};
    view.SizeInBytes = buffer->GetDesc().Size;
    view.StrideInBytes = buffer->GetDesc().Stride;
    view.BufferLocation = buffer->GetAddress();

    mList->IASetVertexBuffers(0, 1, &view);
}

void D3D12CommandList::SetIndexBuffer(IRHIBuffer* buffer)
{
    D3D12_INDEX_BUFFER_VIEW view = {};
    view.BufferLocation = buffer->GetAddress();
    view.Format = DXGI_FORMAT_R32_UINT;
    view.SizeInBytes = buffer->GetDesc().Size;

    mList->IASetIndexBuffer(&view);
}

void D3D12CommandList::SetGraphicsConstants(IRHIGraphicsPipeline* pipeline, const void* data, uint64 size)
{
    mList->SetGraphicsRoot32BitConstants(0, size / 4, data, 0);
}

void D3D12CommandList::SetComputePipeline(IRHIComputePipeline* pipeline)
{
    mList->SetPipelineState(static_cast<D3D12ComputePipeline*>(pipeline)->GetPipelineState());
    mList->SetComputeRootSignature(static_cast<D3D12ComputePipeline*>(pipeline)->GetRootSignature());
}

void D3D12CommandList::SetComputeConstants(IRHIComputePipeline* pipeline, const void* data, uint64 size)
{
    mList->SetComputeRoot32BitConstants(0, size / 4, data, 0);
}

void D3D12CommandList::SetMeshPipeline(IRHIMeshPipeline* pipeline)
{
    D3D12MeshPipeline* d3dPipeline = static_cast<D3D12MeshPipeline*>(pipeline);

    mList->SetPipelineState(d3dPipeline->GetPipelineState());
    mList->SetGraphicsRootSignature(d3dPipeline->GetRootSignature());
}

void D3D12CommandList::SetMeshConstants(IRHIMeshPipeline* pipeline, const void *data, uint64 size)
{
    mList->SetGraphicsRoot32BitConstants(0, size / 4, data, 0);
}

void D3D12CommandList::Draw(uint vertexCount, uint instanceCount, uint firstVertex, uint firstInstance)
{
    mList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void D3D12CommandList::DrawIndexed(uint indexCount, uint instanceCount, uint firstIndex, uint vertexOffset, uint firstInstance)
{
    mList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void D3D12CommandList::Dispatch(uint x, uint y, uint z)
{
    mList->Dispatch(x, y, z);
}

void D3D12CommandList::DispatchMesh(uint x, uint y, uint z)
{
    mList->DispatchMesh(x, y, z);
}

void D3D12CommandList::CopyBufferToBufferFull(IRHIBuffer* dest, IRHIBuffer* src)
{
    mList->CopyResource(static_cast<D3D12Buffer*>(dest)->GetResource(), static_cast<D3D12Buffer*>(src)->GetResource());
}

void D3D12CommandList::CopyBufferToTexture(IRHITexture* dest, IRHIBuffer* src)
{
    auto* srcResource = static_cast<D3D12Buffer*>(src)->GetResource();
    auto* dstResource = static_cast<D3D12Texture*>(dest)->GetResource();
    const RHITextureDesc& textureDesc = dest->GetDesc();
    D3D12_RESOURCE_DESC texDesc = dstResource->GetDesc();
    
    uint64 bufferOffset = 0;
    for (uint mip = 0; mip < textureDesc.MipLevels; ++mip) {
        uint width = max(1u, textureDesc.Width >> mip);
        uint height = max(1u, textureDesc.Height >> mip);
        uint64 rowSizeInBytes;
        uint numRows;
        uint64 rowPitch;
        
        if (IRHITexture::IsBlockFormat(textureDesc.Format)) {
            uint blockWidth = (width + 3) / 4;
            uint blockHeight = (height + 3) / 4;
            // FIX: Use dynamic bytes per block instead of hardcoded 16
            rowSizeInBytes = blockWidth * IRHITexture::BytesPerPixel(textureDesc.Format);
            numRows = blockHeight;
            rowPitch = Align<uint>(rowSizeInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        }
        else {
            rowSizeInBytes = width * IRHITexture::BytesPerPixel(textureDesc.Format);
            numRows = height;
            rowPitch = Align<uint>(rowSizeInBytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
        }
        
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
        footprint.Offset = bufferOffset;
        footprint.Footprint.Format = texDesc.Format;
        footprint.Footprint.Width = width;
        footprint.Footprint.Height = height;
        footprint.Footprint.Depth = 1;
        footprint.Footprint.RowPitch = static_cast<UINT>(rowPitch);
        
        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = dstResource;
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = mip;
        
        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = srcResource;
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint = footprint;
        
        D3D12_BOX* pBox = nullptr; // nullptr means copy full footprint
        mList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, pBox);
        
        bufferOffset += rowPitch * numRows;
    }
}

void D3D12CommandList::CopyTextureToBuffer(IRHIBuffer* dest, IRHITexture* src)
{
    ID3D12Resource* dstResource = static_cast<D3D12Buffer*>(dest)->GetResource();
    ID3D12Resource* srcResource = static_cast<D3D12Texture*>(src)->GetResource();
    D3D12_RESOURCE_DESC desc = srcResource->GetDesc();

    Array<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(desc.MipLevels);
    Array<uint> num_rows(desc.MipLevels);
    Array<uint64_t> row_sizes(desc.MipLevels);
    uint64_t totalSize = 0;

    mParentDevice->GetDevice()->GetCopyableFootprints(
        &desc,
        0, // First subresource
        desc.MipLevels,
        0, // Offset into dstResource
        footprints.data(),
        num_rows.data(),
        row_sizes.data(),
        &totalSize
    );

    for (uint i = 0; i < desc.MipLevels; i++) {
        D3D12_TEXTURE_COPY_LOCATION dstCopy = {};
        dstCopy.pResource = dstResource;
        dstCopy.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        dstCopy.PlacedFootprint = footprints[i];

        D3D12_TEXTURE_COPY_LOCATION srcCopy = {};
        srcCopy.pResource = srcResource;
        srcCopy.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcCopy.SubresourceIndex = i;

        mList->CopyTextureRegion(&dstCopy, 0, 0, 0, &srcCopy, nullptr);
    }
}

void D3D12CommandList::CopyTextureToTexture(IRHITexture* dst, IRHITexture* src)
{
    ID3D12Resource* dstResource = static_cast<D3D12Texture*>(dst)->GetResource();
    ID3D12Resource* srcResource = static_cast<D3D12Texture*>(src)->GetResource();

    mList->CopyResource(dstResource, srcResource);
}

void D3D12CommandList::BuildBLAS(IRHIBLAS* blas, RHIASBuildMode mode)
{
    D3D12BLAS* d3dblas = static_cast<D3D12BLAS*>(blas);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = d3dblas->mInputs;
    buildDesc.DestAccelerationStructureData = d3dblas->mMemory->GetAddress();
    buildDesc.ScratchAccelerationStructureData = d3dblas->mScratch->GetAddress();
    if (mode == RHIASBuildMode::kRefit) {
        buildDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        buildDesc.SourceAccelerationStructureData = d3dblas->mMemory->GetAddress();
    } else {
        buildDesc.SourceAccelerationStructureData = 0;
    }

    mList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
}

void D3D12CommandList::BuildTLAS(IRHITLAS* tlas, RHIASBuildMode mode, uint instanceCount, IRHIBuffer* buffer)
{
    D3D12TLAS* d3dblas = static_cast<D3D12TLAS*>(tlas);

    d3dblas->mInputs.InstanceDescs = buffer->GetAddress();
    d3dblas->mInputs.NumDescs = instanceCount;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs = d3dblas->mInputs;
    buildDesc.DestAccelerationStructureData = d3dblas->mMemory->GetAddress();
    buildDesc.SourceAccelerationStructureData = d3dblas->mMemory->GetAddress();
    buildDesc.ScratchAccelerationStructureData = d3dblas->mScratch->GetAddress();
    if (mode == RHIASBuildMode::kRefit) {
        buildDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
        buildDesc.SourceAccelerationStructureData = d3dblas->mMemory->GetAddress();
    } else {
        buildDesc.SourceAccelerationStructureData = 0;
    }

    mList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
}

void D3D12CommandList::PushMarker(const String& name)
{
    PIXBeginEvent(mList, PIX_COLOR_DEFAULT, name.data());
}

void D3D12CommandList::PopMarker()
{
    PIXEndEvent(mList);
}

void D3D12CommandList::BeginImGui()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void D3D12CommandList::EndImGui()
{
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), mList);
}

D3D12_BARRIER_SYNC D3D12CommandList::ToD3D12BarrierSync(RHIPipelineStage stage)
{
    using enum RHIPipelineStage;
    D3D12_BARRIER_SYNC sync = D3D12_BARRIER_SYNC_NONE;

    if (Any(stage & kAllCommands)) sync |= D3D12_BARRIER_SYNC_ALL;
    if (Any(stage & kAllGraphics)) sync |= D3D12_BARRIER_SYNC_ALL_SHADING;
    if (Any(stage & kDrawIndirect)) sync |= D3D12_BARRIER_SYNC_EXECUTE_INDIRECT;
    if (Any(stage & kVertexInput)) sync |= D3D12_BARRIER_SYNC_DRAW;
    if (Any(stage & kIndexInput)) sync |= D3D12_BARRIER_SYNC_INDEX_INPUT;
    if (Any(stage & kVertexShader)) sync |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
    if (Any(stage & kHullShader) || Any(stage & kDomainShader) || Any(stage & kGeometryShader)) sync |= D3D12_BARRIER_SYNC_NON_PIXEL_SHADING;
    if (Any(stage & kPixelShader)) sync |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
    if (Any(stage & kComputeShader)) sync |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
    if (Any(stage & kRayTracingShader)) sync |= D3D12_BARRIER_SYNC_RAYTRACING;
    if (Any(stage & kEarlyFragmentTests) || Any(stage & kLateFragmentTests)) sync |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    if (Any(stage & kColorAttachmentOutput)) sync |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    if (Any(stage & kResolve)) sync |= D3D12_BARRIER_SYNC_RESOLVE;
    if (Any(stage & kCopy)) sync |= D3D12_BARRIER_SYNC_COPY;
    if (Any(stage & kAccelStructureWrite)) sync |= D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE;
    if (Any(stage & kBottomOfPipe)) sync |= D3D12_BARRIER_SYNC_ALL_SHADING;
    if (Any(stage & kTopOfPipe)) sync |= D3D12_BARRIER_SYNC_ALL_SHADING;

    return sync;
}

D3D12_BARRIER_ACCESS D3D12CommandList::ToD3D12BarrierAccess(RHIResourceAccess access)
{
    using enum RHIResourceAccess;
    D3D12_BARRIER_ACCESS flags = D3D12_BARRIER_ACCESS_NO_ACCESS;

    if (Any(access & kIndirectCommandRead)) flags = D3D12_BARRIER_ACCESS_INDIRECT_ARGUMENT;
    if (Any(access & kVertexBufferRead)) flags = D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
    if (Any(access & kIndexBufferRead)) flags = D3D12_BARRIER_ACCESS_INDEX_BUFFER;
    if (Any(access & kConstantBufferRead)) flags = D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
    if (Any(access & kShaderRead)) flags = D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    if (Any(access & kShaderWrite)) flags = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
    if (Any(access & kColorAttachmentRead)) flags = D3D12_BARRIER_ACCESS_RENDER_TARGET; // Read-only RT
    if (Any(access & kColorAttachmentWrite)) flags = D3D12_BARRIER_ACCESS_RENDER_TARGET;
    if (Any(access & kDepthStencilRead)) flags = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    if (Any(access & kDepthStencilWrite)) flags = D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    if (Any(access & kTransferRead)) flags = D3D12_BARRIER_ACCESS_COPY_SOURCE;
    if (Any(access & kTransferWrite)) flags = D3D12_BARRIER_ACCESS_COPY_DEST;
    if (Any(access & kHostRead) || Any(access & kHostWrite)) flags = D3D12_BARRIER_ACCESS_NO_ACCESS;
    if (Any(access & kMemoryRead)) flags = D3D12_BARRIER_ACCESS_COMMON;
    if (Any(access & kMemoryWrite)) flags = D3D12_BARRIER_ACCESS_COMMON;
    if (Any(access & kAccelerationStructureRead)) flags = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ;
    if (Any(access & kAccelerationStructureWrite)) flags = D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE;

    return flags;
}

D3D12_BARRIER_LAYOUT D3D12CommandList::ToD3D12BarrierLayout(RHIResourceLayout layout)
{
    switch (layout)
    {
    case RHIResourceLayout::kUndefined:
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    case RHIResourceLayout::kGeneral:
        return D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
    case RHIResourceLayout::kReadOnly:
        return D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
    case RHIResourceLayout::kColorAttachment:
        return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
    case RHIResourceLayout::kDepthStencilReadOnly:
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
    case RHIResourceLayout::kDepthStencilWrite:
        return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
    case RHIResourceLayout::kTransferSrc:
        return D3D12_BARRIER_LAYOUT_COPY_SOURCE;
    case RHIResourceLayout::kTransferDst:
        return D3D12_BARRIER_LAYOUT_COPY_DEST;
    case RHIResourceLayout::kPresent:
        return D3D12_BARRIER_LAYOUT_PRESENT;
    default:
        return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }
    return D3D12_BARRIER_LAYOUT_UNDEFINED;
}
