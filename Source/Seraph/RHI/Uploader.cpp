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

    SERAPH_WHATEVER("Initialized uploader");
}

void Uploader::Shutdown()
{
    for (auto& request : sData.Requests) {
        if (request.StagingBuffer) delete request.StagingBuffer;
    }
    sData.Requests.clear();
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
    memcpy(ptr, data, size);
    request.StagingBuffer->Unmap();

    sData.Requests.push_back(request);

    if (sData.UploadBatchSize >= MAX_BATCH_SIZE)
        Flush();
}

void Uploader::Flush()
{
    if (sData.Requests.empty())
        return;

    SERAPH_WHATEVER("Flushing uploader, batch size %llu bytes", sData.UploadBatchSize);

    sData.CommandBuffer = sData.CopyQueue->CreateCommandBuffer(true);
    sData.CommandBuffer->Begin();
    for (auto& request : sData.Requests) {
        switch (request.Type) {
            case UploadRequestType::kBufferCPUToGPU: {
                RHIBufferDesc dstDesc = request.DstBuffer->GetDesc();

                RHIBufferBarrier dstBarrier(request.DstBuffer);
                dstBarrier.SourceStage = RHIPipelineStage::kNone;
                dstBarrier.DestStage = RHIPipelineStage::kCopy;
                dstBarrier.SourceAccess = RHIResourceAccess::kNone;
                dstBarrier.DestAccess = RHIResourceAccess::kTransferWrite;

                RHIBufferBarrier stagingBarrier(request.DstBuffer);
                stagingBarrier.SourceStage = RHIPipelineStage::kNone;
                stagingBarrier.DestStage = RHIPipelineStage::kCopy;
                stagingBarrier.SourceAccess = RHIResourceAccess::kNone;
                stagingBarrier.DestAccess = RHIResourceAccess::kTransferRead;

                RHIBarrierGroup firstGroup = {};
                firstGroup.BufferBarriers = { dstBarrier, stagingBarrier };

                RHIBufferBarrier dstBarrierAfter(request.DstBuffer);
                dstBarrierAfter.SourceStage = RHIPipelineStage::kCopy;
                dstBarrierAfter.SourceAccess = RHIResourceAccess::kTransferWrite;        
                if (Any(dstDesc.Usage & RHIBufferUsage::kVertex)) dstBarrierAfter.DestAccess = RHIResourceAccess::kVertexBufferRead; dstBarrierAfter.DestStage = RHIPipelineStage::kVertexShader;
                if (Any(dstDesc.Usage & RHIBufferUsage::kIndex)) dstBarrierAfter.DestAccess = RHIResourceAccess::kIndexBufferRead; dstBarrierAfter.DestStage = RHIPipelineStage::kVertexShader;
                if (Any(dstDesc.Usage & RHIBufferUsage::kConstant)) dstBarrierAfter.DestAccess = RHIResourceAccess::kConstantBufferRead; dstBarrierAfter.DestStage = RHIPipelineStage::kAllGraphics;

                sData.CommandBuffer->BarrierGroup(firstGroup);
                sData.CommandBuffer->CopyBufferToBufferFull(request.DstBuffer, request.StagingBuffer);
                sData.CommandBuffer->Barrier(dstBarrierAfter);
                break;
            }
        }
    }
    sData.CommandBuffer->End();
    sData.CopyQueue->SubmitAndFlushCommandBuffer(sData.CommandBuffer);

    for (auto& request : sData.Requests) {
        if (request.StagingBuffer) delete request.StagingBuffer;
    }
    sData.Requests.clear();
    delete sData.CommandBuffer;
}
