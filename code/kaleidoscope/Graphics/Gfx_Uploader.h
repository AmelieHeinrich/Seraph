//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-17 23:46:18
//

#pragma once

#include <KernelGPU/KGPU_Device.h>

namespace Gfx
{
    class Uploader
    {
    public:
        static void EnqueueTLASBuild(KGPU::ITLAS* tlas, KGPU::IBuffer* instanceBuffer, uint instanceCount);
        static void EnqueueBLASBuild(KGPU::IBLAS* blas);
        static void EnqueueTextureUploadRaw(const void* data, uint64 size, KGPU::ITexture* texture, bool bufferHasMips = true, KGPU::ICommandList* cmdList = nullptr);
        static void EnqueueBufferUpload(const void* data, uint64 size, KGPU::IBuffer* buffer);
    };
}
