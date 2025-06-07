//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 13:02:32
//

/*
    This class is used to batch CPU to GPU data uploads, whether it's textures, buffers, or even acceleration structures build on app startup.
    They batch them and create the necessary staging buffers before flushing all the commands with Uploader::Flush.
*/

#pragma once

#include "Device.h"

class Uploader
{
public:
    static void Initialize(IRHIDevice* device, IRHICommandQueue* copyQueue);
    static void Shutdown();

    static void EnqueueTLASBuild(IRHITLAS* tlas, IRHIBuffer* instanceBuffer, uint instanceCount);
    static void EnqueueBLASBuild(IRHIBLAS* blas);
    static void EnqueueTextureUploadRaw(const void* data, uint64 size, IRHITexture* texture);
    static void EnqueueBufferUpload(const void* data, uint64 size, IRHIBuffer* buffer);
    static void Flush();
private:
    static constexpr uint64 MAX_BATCH_SIZE = MEGABYTES(512);

    enum class UploadRequestType
    {
        kBufferCPUToGPU,
        kTextureCPUToGPU,
        kBLASBuild,
        kTLASBuild
    };

    struct UploadRequest
    {
        UploadRequestType Type;

        IRHIBLAS* BLAS;
        IRHITLAS* TLAS;
        IRHITexture* DstTexture;
        IRHIBuffer* DstBuffer;
        IRHIBuffer* StagingBuffer;

        IRHIBuffer* InstanceBuffer;
        uint InstanceCount;
    };

    static struct PrivateData {
        IRHIDevice* Device;
        IRHICommandQueue* CopyQueue;
        
        IRHICommandList* CommandBuffer;
        Array<UploadRequest> Requests;

        int BufferRequests;
        int UploadBatchSize;
    } sData;
};
