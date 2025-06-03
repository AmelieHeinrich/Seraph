//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 13:06:50
//

#include "Uploader.h"

Uploader::PrivateData Uploader::sData;

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

// This function is fucking beautiful actually and I'm very proud of it :3
void Uploader::EnqueueTextureUploadRaw(const void* data, uint64 size, IRHITexture* texture)
{
    RHITextureDesc desc = texture->GetDesc();
    uint mipLevels = desc.MipLevels;
    uint baseWidth = desc.Width;
    uint baseHeight = desc.Height;

    Array<uint64> mipRowSize(mipLevels);
    Array<uint64> mipAlignedPitch(mipLevels);
    Array<uint64> mipOffset(mipLevels);

    uint64 totalStagingSize = 0;
    const uint8* srcPtr = reinterpret_cast<const uint8*>(data);

    for (uint mip = 0; mip < mipLevels; mip++) {
        uint mipWidth = std::max(1u, baseWidth >> mip);
        uint mipHeight = std::max(1u, baseHeight >> mip);

        if (IRHITexture::IsBlockFormat(desc.Format)) {
            uint blocksWide = (mipWidth + 3) / 4;
            uint blocksHigh = (mipHeight + 3) / 4;
            uint bytesPerBlock = IRHITexture::BytesPerPixel(desc.Format);

            uint64 rowSize = blocksWide * bytesPerBlock;
            mipRowSize[mip] = rowSize;

            uint64 alignedPitch = Align<uint64>(rowSize, TEXTURE_ROW_PITCH_ALIGNMENT);
            mipAlignedPitch[mip] = alignedPitch;

            mipOffset[mip] = totalStagingSize;
            totalStagingSize += alignedPitch * blocksHigh;
        } else {
            uint bpp = IRHITexture::BytesPerPixel(desc.Format);
            uint64 rowSize = mipWidth * bpp;
            mipRowSize[mip] = rowSize;

            uint64 alignedPitch = Align<uint64>(rowSize, TEXTURE_ROW_PITCH_ALIGNMENT);
            mipAlignedPitch[mip] = alignedPitch;

            mipOffset[mip] = totalStagingSize;
            totalStagingSize += alignedPitch * mipHeight;
        }
    }

    RHIBufferDesc stagingDesc = {};
    stagingDesc.Size  = totalStagingSize;
    stagingDesc.Usage = RHIBufferUsage::kStaging;

    UploadRequest request = {};
    request.Type = UploadRequestType::kTextureCPUToGPU;
    request.DstTexture = texture;
    request.StagingBuffer = sData.Device->CreateBuffer(stagingDesc);

    void* mappedVoid = request.StagingBuffer->Map();
    uint8* dstBase = reinterpret_cast<uint8_t*>(mappedVoid);
    for (uint mip = 0; mip < mipLevels; mip++) {
        uint mipWidth = std::max(1u, baseWidth >> mip);
        uint mipHeight = std::max(1u, baseHeight >> mip);
        uint8* dstMipRow = dstBase + mipOffset[mip];

        if (IRHITexture::IsBlockFormat(desc.Format)) {
            uint blocksHigh = (mipHeight + 3) / 4;
            for (uint by = 0; by < blocksHigh; by++) {
                SafeMemcpy(
                    dstMipRow + by * mipAlignedPitch[mip],
                    srcPtr + by * mipRowSize[mip],
                    mipRowSize[mip]
                );
            }

            srcPtr += mipRowSize[mip] * blocksHigh;
        } else {
            for (uint y = 0; y < mipHeight; y++) {
                SafeMemcpy(
                    dstMipRow + y * mipAlignedPitch[mip],
                    srcPtr + y * mipRowSize[mip],
                    mipRowSize[mip]
                );
            }
            srcPtr += mipRowSize[mip] * mipHeight;
        }
    }
    request.StagingBuffer->Unmap();

    sData.Requests.push_back(std::move(request));
    sData.UploadBatchSize += totalStagingSize;
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
