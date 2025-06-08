//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 13:06:50
//

#include "Uploader.h"

Uploader::PrivateData Uploader::sData;

struct MipLevelInfo
{
    uint32 Width, Height;
    uint64 BufferOffset;
    uint64 RowPitch;
};

void Uploader::Initialize(IRHIDevice* device, IRHICommandQueue* copyQueue)
{
    sData = {};
    sData.Device = device;
    sData.CopyQueue = copyQueue;
    sData.UploadBatchSize = 0;

    SERAPH_WHATEVER("Initialized uploader");
}

void Uploader::Shutdown()
{
    for (auto& request : sData.Requests) {
        if (request.StagingBuffer) delete request.StagingBuffer;
    }
    sData.Requests.clear();
}

void Uploader::EnqueueTLASBuild(IRHITLAS* tlas, IRHIBuffer* instanceBuffer, uint instanceCount)
{
    UploadRequest request = {};
    request.Type = UploadRequestType::kTLASBuild;
    request.TLAS = tlas;
    request.InstanceBuffer = instanceBuffer;
    request.InstanceCount = instanceCount;

    sData.Requests.push_back(std::move(request));
    sData.UploadBatchSize += MAX_TLAS_INSTANCES; // Approximate
    if (sData.UploadBatchSize >= MAX_BATCH_SIZE)
        Flush();
}

void Uploader::EnqueueBLASBuild(IRHIBLAS* blas)
{
    UploadRequest request = {};
    request.Type = UploadRequestType::kBLASBuild;
    request.BLAS = blas;

    sData.Requests.push_back(std::move(request));
    sData.UploadBatchSize += blas->GetDesc().VertexCount * 2; // Approximate
    if (sData.UploadBatchSize >= MAX_BATCH_SIZE)
        Flush();
}

void Uploader::EnqueueTextureUploadRaw(const void* data, uint64 size, IRHITexture* texture)
{
    RHITextureDesc desc = texture->GetDesc();
    uint mipLevels = desc.MipLevels;
    uint baseWidth = desc.Width;
    uint baseHeight = desc.Height;
    Array<MipLevelInfo> mips;
    uint64 totalBufferSize = 0;
    // 1. Layout mip levels
    for (uint32 i = 0; i < mipLevels; i++) {
        MipLevelInfo mip = {};
        mip.Width = std::max(1u, baseWidth >> i);
        mip.Height = std::max(1u, baseHeight >> i);
        
        if (IRHITexture::IsBlockFormat(desc.Format)) {
            uint blockWidth = (mip.Width + 3) / 4;
            uint blockHeight = (mip.Height + 3) / 4; // FIX: Calculate block height
            // FIX: Use dynamic bytes per block instead of hardcoded 16
            mip.RowPitch = Align<uint>(blockWidth * IRHITexture::BytesPerPixel(desc.Format), sData.Device->GetOptimalRowPitchAlignment());
            // FIX: Use block height for buffer size calculation
            totalBufferSize += mip.RowPitch * blockHeight;
        } else {
            mip.RowPitch = Align<uint>(mip.Width * IRHITexture::BytesPerPixel(desc.Format), sData.Device->GetOptimalRowPitchAlignment());
            totalBufferSize += mip.RowPitch * mip.Height;
        }
        mip.BufferOffset = totalBufferSize - (IRHITexture::IsBlockFormat(desc.Format) ? 
            mip.RowPitch * ((mip.Height + 3) / 4) : mip.RowPitch * mip.Height);
        mips.push_back(mip);
    }
    // 2. Allocate staging buffer
    RHIBufferDesc stagingDesc = {};
    stagingDesc.Size = totalBufferSize;
    stagingDesc.Usage = RHIBufferUsage::kStaging;
    UploadRequest request = {};
    request.Type = UploadRequestType::kTextureCPUToGPU;
    request.DstTexture = texture;
    request.StagingBuffer = sData.Device->CreateBuffer(stagingDesc);
    // 3. Copy data into staging buffer
    void* mappedVoid = request.StagingBuffer->Map();
    uint8* dstBase = reinterpret_cast<uint8*>(mappedVoid);
    const uint8* srcPtr = reinterpret_cast<const uint8*>(data);
    uint64 srcOffset = 0;
    for (uint mip = 0; mip < mipLevels; mip++) {
        const auto& mipInfo = mips[mip];
        uint32 srcWidth = mipInfo.Width;
        uint32 srcHeight = mipInfo.Height;
        uint32 rowPitch = mipInfo.RowPitch;
        if (IRHITexture::IsBlockFormat(desc.Format)) {
            uint32 blockWidth = (srcWidth + 3) / 4;
            uint32 blockHeight = (srcHeight + 3) / 4;
            // FIX: Use dynamic bytes per block instead of hardcoded 16
            uint32 rowSize = blockWidth * IRHITexture::BytesPerPixel(desc.Format);
            for (uint32 y = 0; y < blockHeight; ++y) {
                const uint8* srcRow = srcPtr + srcOffset;
                uint8* dstRow = dstBase + mipInfo.BufferOffset + y * rowPitch;
                SafeMemcpy(dstRow, srcRow, rowSize);
                srcOffset += rowSize;
            }
        }
        else {
            uint32 rowSize = srcWidth * IRHITexture::BytesPerPixel(desc.Format);
            for (uint32 y = 0; y < srcHeight; ++y) {
                const uint8* srcRow = srcPtr + srcOffset;
                uint8* dstRow = dstBase + mipInfo.BufferOffset + y * rowPitch;
                SafeMemcpy(dstRow, srcRow, rowSize);
                srcOffset += rowSize;
            }
        }
    }
    request.StagingBuffer->Unmap();
    sData.Requests.push_back(std::move(request));
    sData.UploadBatchSize += totalBufferSize;
    if (sData.UploadBatchSize >= MAX_BATCH_SIZE)
        Flush();
}

void Uploader::EnqueueBufferUpload(const void* data, uint64 size, IRHIBuffer* buffer)
{
    sData.UploadBatchSize += size;

    RHIBufferDesc stagingBufferDesc = {};
    stagingBufferDesc.Size = size;
    stagingBufferDesc.Usage = RHIBufferUsage::kStaging;

    UploadRequest request = {};
    request.Type = UploadRequestType::kBufferCPUToGPU;
    request.DstBuffer = buffer;
    request.StagingBuffer = sData.Device->CreateBuffer(stagingBufferDesc);

    void* ptr = request.StagingBuffer->Map();
    SafeMemcpy(ptr, data, size);
    request.StagingBuffer->Unmap();

    sData.Requests.push_back(std::move(request));

    if (sData.UploadBatchSize >= MAX_BATCH_SIZE)
        Flush();
}

void Uploader::Flush()
{
    if (sData.Requests.empty())
        return;

    SERAPH_WHATEVER("Flushing uploader, batch size %d bytes", sData.UploadBatchSize);

    sData.CommandBuffer = sData.CopyQueue->CreateCommandBuffer(true);
    sData.CommandBuffer->Begin();
    for (auto& request : sData.Requests) {
        switch (request.Type) {
            case UploadRequestType::kBufferCPUToGPU: {
                RHIBufferDesc dstDesc = request.DstBuffer->GetDesc();

                RHIBufferBarrier dstBarrier(request.DstBuffer);
                dstBarrier.SourceStage = RHIPipelineStage::kAllCommands;
                dstBarrier.DestStage = RHIPipelineStage::kCopy;
                dstBarrier.SourceAccess = RHIResourceAccess::kNone;
                dstBarrier.DestAccess = RHIResourceAccess::kTransferWrite;

                RHIBufferBarrier stagingBarrier(request.DstBuffer);
                stagingBarrier.SourceStage = RHIPipelineStage::kAllCommands;
                stagingBarrier.DestStage = RHIPipelineStage::kCopy;
                stagingBarrier.SourceAccess = RHIResourceAccess::kNone;
                stagingBarrier.DestAccess = RHIResourceAccess::kTransferRead;

                RHIBarrierGroup firstGroup = {};
                firstGroup.BufferBarriers = { dstBarrier, stagingBarrier };

                RHIBufferBarrier dstBarrierAfter(request.DstBuffer);
                dstBarrierAfter.SourceStage = RHIPipelineStage::kCopy;
                dstBarrierAfter.SourceAccess = RHIResourceAccess::kTransferWrite;
                if (Any(dstDesc.Usage & RHIBufferUsage::kVertex)) {
                    dstBarrierAfter.DestAccess = RHIResourceAccess::kVertexBufferRead;
                    dstBarrierAfter.DestStage = RHIPipelineStage::kVertexInput;
                }
                if (Any(dstDesc.Usage & RHIBufferUsage::kIndex)) {
                    dstBarrierAfter.DestAccess = RHIResourceAccess::kIndexBufferRead;
                    dstBarrierAfter.DestStage = RHIPipelineStage::kVertexInput;
                }
                if (Any(dstDesc.Usage & RHIBufferUsage::kConstant)) {
                    dstBarrierAfter.DestAccess = RHIResourceAccess::kConstantBufferRead;
                    dstBarrierAfter.DestStage = RHIPipelineStage::kAllCommands;
                }
                if (Any(dstDesc.Usage & RHIBufferUsage::kShaderRead)) {
                    dstBarrierAfter.DestAccess = RHIResourceAccess::kShaderRead;
                    dstBarrierAfter.DestStage = RHIPipelineStage::kAllGraphics;
                }
                if (Any(dstDesc.Usage & RHIBufferUsage::kShaderWrite)) {
                    dstBarrierAfter.DestAccess = RHIResourceAccess::kShaderWrite;
                    dstBarrierAfter.DestStage = RHIPipelineStage::kAllGraphics;
                }

                sData.CommandBuffer->BarrierGroup(firstGroup);
                sData.CommandBuffer->CopyBufferToBufferFull(request.DstBuffer, request.StagingBuffer);
                sData.CommandBuffer->Barrier(dstBarrierAfter);
                break;
            }
            case UploadRequestType::kTextureCPUToGPU: {
                RHITextureBarrier dstBarrier(request.DstTexture);
                dstBarrier.SourceStage = RHIPipelineStage::kAllCommands;
                dstBarrier.DestStage = RHIPipelineStage::kCopy;
                dstBarrier.SourceAccess = RHIResourceAccess::kNone;
                dstBarrier.DestAccess = RHIResourceAccess::kTransferWrite;
                dstBarrier.NewLayout = RHIResourceLayout::kTransferDst;
                dstBarrier.BaseMipLevel = 0;
                dstBarrier.LevelCount = request.DstTexture->GetDesc().MipLevels;

                RHIBufferBarrier stagingBarrier(request.StagingBuffer);
                stagingBarrier.SourceStage = RHIPipelineStage::kAllCommands;
                stagingBarrier.DestStage = RHIPipelineStage::kCopy;
                stagingBarrier.SourceAccess = RHIResourceAccess::kNone;
                stagingBarrier.DestAccess = RHIResourceAccess::kTransferRead;

                RHIBarrierGroup firstGroup = {};
                firstGroup.BufferBarriers = { stagingBarrier };
                firstGroup.TextureBarriers = { dstBarrier };

                RHITextureBarrier dstBarrierAfter(request.DstTexture);
                dstBarrierAfter.SourceStage = RHIPipelineStage::kCopy;
                dstBarrierAfter.DestStage = RHIPipelineStage::kAllGraphics;
                dstBarrierAfter.SourceAccess = RHIResourceAccess::kTransferWrite;
                dstBarrierAfter.DestAccess = RHIResourceAccess::kShaderRead;
                dstBarrierAfter.NewLayout = RHIResourceLayout::kReadOnly;
                dstBarrierAfter.BaseMipLevel = 0;
                dstBarrierAfter.LevelCount = request.DstTexture->GetDesc().MipLevels;

                sData.CommandBuffer->BarrierGroup(firstGroup);
                sData.CommandBuffer->CopyBufferToTexture(request.DstTexture, request.StagingBuffer);
                sData.CommandBuffer->Barrier(dstBarrierAfter);
                break;
            }
            case UploadRequestType::kBLASBuild: {
                RHIBufferBarrier beforeBarrier(request.BLAS->GetMemory());
                beforeBarrier.SourceAccess = RHIResourceAccess::kTransferWrite;
                beforeBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureWrite;
                beforeBarrier.SourceStage = RHIPipelineStage::kCopy;
                beforeBarrier.DestStage = RHIPipelineStage::kAccelStructureWrite;

                RHIBufferBarrier afterBarrier(request.BLAS->GetMemory());
                afterBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureWrite;
                afterBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureRead;
                afterBarrier.SourceStage = RHIPipelineStage::kAccelStructureWrite;
                afterBarrier.DestStage = RHIPipelineStage::kRayTracingShader;

                sData.CommandBuffer->Barrier(beforeBarrier);
                sData.CommandBuffer->BuildBLAS(request.BLAS, RHIASBuildMode::kRebuild);
                sData.CommandBuffer->Barrier(afterBarrier);
                break;
            }
            case UploadRequestType::kTLASBuild: {
                RHIBufferBarrier beforeBarrier(request.TLAS->GetMemory());
                beforeBarrier.SourceAccess = RHIResourceAccess::kTransferWrite;
                beforeBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureWrite;
                beforeBarrier.SourceStage = RHIPipelineStage::kCopy;
                beforeBarrier.DestStage = RHIPipelineStage::kAccelStructureWrite;

                RHIBufferBarrier afterBarrier(request.TLAS->GetMemory());
                afterBarrier.SourceAccess = RHIResourceAccess::kAccelerationStructureWrite;
                afterBarrier.DestAccess = RHIResourceAccess::kAccelerationStructureRead;
                afterBarrier.SourceStage = RHIPipelineStage::kAccelStructureWrite;
                afterBarrier.DestStage = RHIPipelineStage::kRayTracingShader;

                sData.CommandBuffer->Barrier(beforeBarrier);
                sData.CommandBuffer->BuildTLAS(request.TLAS, RHIASBuildMode::kRebuild, request.InstanceCount, request.InstanceBuffer);
                sData.CommandBuffer->Barrier(afterBarrier);
                break;
            }
        }
    }
    sData.CommandBuffer->End();
    sData.CopyQueue->SubmitAndFlushCommandBuffer(sData.CommandBuffer);
    sData.UploadBatchSize = 0;

    for (auto& request : sData.Requests) {
        if (request.StagingBuffer) delete request.StagingBuffer;
    }
    sData.Requests.clear();
    delete sData.CommandBuffer;
}
