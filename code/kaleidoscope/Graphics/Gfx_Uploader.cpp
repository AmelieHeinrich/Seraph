//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-17 23:48:17
//

#include "Gfx_Uploader.h"
#include "Gfx_Manager.h"
#include "Gfx_CommandListRecycler.h"
#include "Gfx_TempResourceStorage.h"

namespace Gfx
{
    struct MipLevelInfo
    {
        uint32 Width, Height;
        uint64 BufferOffset;
        uint64 RowPitch;
    };

    void Uploader::EnqueueTLASBuild(KGPU::ITLAS* tlas, KGPU::IBuffer* instanceBuffer, uint instanceCount)
    {
        KGPU::ICommandList* cmdList = CommandListRecycler::RequestCommandList();

        KGPU::BufferBarrier beforeBarrier(tlas->GetMemory());
        beforeBarrier.SourceAccess = KGPU::ResourceAccess::kTransferWrite;
        beforeBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        beforeBarrier.SourceStage = KGPU::PipelineStage::kCopy;
        beforeBarrier.DestStage = KGPU::PipelineStage::kAccelStructureWrite;

        KGPU::BufferBarrier afterBarrier(tlas->GetMemory());
        afterBarrier.SourceAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        afterBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureRead;
        afterBarrier.SourceStage = KGPU::PipelineStage::kAccelStructureWrite;
        afterBarrier.DestStage = KGPU::PipelineStage::kRayTracingShader;

        cmdList->Barrier(beforeBarrier);
        cmdList->BuildTLAS(tlas, KGPU::ASBuildMode::kRebuild, instanceCount, instanceBuffer);
        cmdList->Barrier(afterBarrier);
    }

    void Uploader::EnqueueBLASBuild(KGPU::IBLAS* blas)
    {
        KGPU::ICommandList* cmdList = CommandListRecycler::RequestCommandList();

        KGPU::BufferBarrier beforeBarrier(blas->GetMemory());
        beforeBarrier.SourceAccess = KGPU::ResourceAccess::kTransferWrite;
        beforeBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        beforeBarrier.SourceStage = KGPU::PipelineStage::kCopy;
        beforeBarrier.DestStage = KGPU::PipelineStage::kAccelStructureWrite;

        KGPU::BufferBarrier afterBarrier(blas->GetMemory());
        afterBarrier.SourceAccess = KGPU::ResourceAccess::kAccelerationStructureWrite;
        afterBarrier.DestAccess = KGPU::ResourceAccess::kAccelerationStructureRead;
        afterBarrier.SourceStage = KGPU::PipelineStage::kAccelStructureWrite;
        afterBarrier.DestStage = KGPU::PipelineStage::kRayTracingShader;

        cmdList->Barrier(beforeBarrier);
        cmdList->BuildBLAS(blas, KGPU::ASBuildMode::kRebuild);
        cmdList->Barrier(afterBarrier);
    }

    void Uploader::EnqueueTextureUploadRaw(const void* data, uint64 size, KGPU::ITexture* texture, bool bufferHasMips, KGPU::ICommandList* list)
    {
        KGPU::TextureDesc desc = texture->GetDesc();
        uint mipLevels = bufferHasMips ? desc.MipLevels : 1;
        uint baseWidth = desc.Width;
        uint baseHeight = desc.Height;
        KC::Array<MipLevelInfo> mips;

        // Get buffer-image granularity for proper alignment
        uint64 bufferImageGranularity = Manager::GetDevice()->GetBufferImageGranularity();
        uint64 currentOffset = 0;

        // 1. Layout mip levels with proper alignment
        for (uint32 i = 0; i < mipLevels; i++) {
            MipLevelInfo mip = {};
            mip.Width = std::max(1u, baseWidth >> i);
            mip.Height = std::max(1u, baseHeight >> i);

            uint64 mipSize;
            if (KGPU::ITexture::IsBlockFormat(desc.Format)) {
                uint blockWidth = (mip.Width + 3) / 4;
                uint blockHeight = (mip.Height + 3) / 4;
                // MUST match VulkanCommandList::CopyBufferToTexture calculation
                mip.RowPitch = KOS::Align<uint>(blockWidth * KGPU::ITexture::BytesPerPixel(desc.Format), Manager::GetDevice()->GetOptimalRowPitchAlignment());
                mipSize = mip.RowPitch * blockHeight;
            } else {
                // MUST match VulkanCommandList::CopyBufferToTexture calculation  
                mip.RowPitch = KOS::Align<uint>(mip.Width * KGPU::ITexture::BytesPerPixel(desc.Format), Manager::GetDevice()->GetOptimalRowPitchAlignment());
                mipSize = mip.RowPitch * mip.Height;
            }

            // Set the buffer offset (aligned to granularity)
            mip.BufferOffset = currentOffset;
            currentOffset += mipSize;

            // Align for next mip level (except for the last one)
            if (i < mipLevels - 1) {
                currentOffset = KOS::Align<uint64>(currentOffset, bufferImageGranularity);
            }

            mips.push_back(mip);
        }

        uint64 totalBufferSize = currentOffset;

        // 2. Allocate staging buffer with the correctly calculated size
        KGPU::BufferDesc stagingDesc = {};
        stagingDesc.Size = totalBufferSize; // No need for extra alignment here
        stagingDesc.Usage = KGPU::BufferUsage::kStaging;

        KGPU::IBuffer* stagingBuffer = TempResourceStorage::CreateBuffer(stagingDesc);
        stagingBuffer->SetName("Staging Buffer");

        // 3. Copy data into staging buffer
        void* mappedVoid = stagingBuffer->Map();
        uint8* dstBase = reinterpret_cast<uint8*>(mappedVoid);
        const uint8* srcPtr = reinterpret_cast<const uint8*>(data);
        uint64 srcOffset = 0;

        for (uint mip = 0; mip < mipLevels; mip++) {
            const auto& mipInfo = mips[mip];
            uint32 srcWidth = mipInfo.Width;
            uint32 srcHeight = mipInfo.Height;
            uint32 rowPitch = mipInfo.RowPitch;

            if (KGPU::ITexture::IsBlockFormat(desc.Format)) {
                uint32 blockWidth = (srcWidth + 3) / 4;
                uint32 blockHeight = (srcHeight + 3) / 4;

                uint32 rowSize = blockWidth * KGPU::ITexture::BytesPerPixel(desc.Format);
                for (uint32 y = 0; y < blockHeight; ++y) {
                    const uint8* srcRow = srcPtr + srcOffset;
                    uint8* dstRow = dstBase + mipInfo.BufferOffset + y * rowPitch;
                    memcpy(dstRow, srcRow, rowSize);
                    srcOffset += rowSize;
                }
            }
            else {
                uint32 rowSize = srcWidth * KGPU::ITexture::BytesPerPixel(desc.Format);
                for (uint32 y = 0; y < srcHeight; ++y) {
                    const uint8* srcRow = srcPtr + srcOffset;
                    uint8* dstRow = dstBase + mipInfo.BufferOffset + y * rowPitch;
                    memcpy(dstRow, srcRow, rowSize);
                    srcOffset += rowSize;
                }
            }
        }
        stagingBuffer->Unmap();

        // Upload
        KGPU::ICommandList* cmdList = list == nullptr ? CommandListRecycler::RequestCommandList() : list;

        KGPU::TextureBarrier dstBarrier(texture);
        dstBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
        dstBarrier.DestStage = KGPU::PipelineStage::kCopy;
        dstBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        dstBarrier.DestAccess = KGPU::ResourceAccess::kTransferWrite;
        dstBarrier.NewLayout = KGPU::ResourceLayout::kTransferDst;
        dstBarrier.BaseMipLevel = 0;
        dstBarrier.LevelCount = texture->GetDesc().MipLevels;

        KGPU::BufferBarrier stagingBarrier(stagingBuffer);
        stagingBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
        stagingBarrier.DestStage = KGPU::PipelineStage::kCopy;
        stagingBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        stagingBarrier.DestAccess = KGPU::ResourceAccess::kTransferRead;

        KGPU::BarrierGroup firstGroup = {};
        firstGroup.BufferBarriers = { stagingBarrier };
        firstGroup.TextureBarriers = { dstBarrier };

        KGPU::TextureBarrier dstBarrierAfter(texture);
        dstBarrierAfter.SourceStage = KGPU::PipelineStage::kCopy;
        dstBarrierAfter.DestStage = KGPU::PipelineStage::kAllGraphics;
        dstBarrierAfter.SourceAccess = KGPU::ResourceAccess::kTransferWrite;
        dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kShaderRead;
        dstBarrierAfter.NewLayout = KGPU::ResourceLayout::kReadOnly;
        dstBarrierAfter.BaseMipLevel = 0;
        dstBarrierAfter.LevelCount = texture->GetDesc().MipLevels;

        cmdList->Barrier(firstGroup);
        cmdList->CopyBufferToTexture(texture, stagingBuffer, bufferHasMips);
        cmdList->Barrier(dstBarrierAfter);
    }

    void Uploader::EnqueueBufferUpload(const void* data, uint64 size, KGPU::IBuffer* buffer)
    {
        KGPU::BufferDesc stagingBufferDesc = {};
        stagingBufferDesc.Size = size;
        stagingBufferDesc.Usage = KGPU::BufferUsage::kStaging;

        KGPU::IBuffer* stagingBuffer = TempResourceStorage::CreateBuffer(stagingBufferDesc);

        void* ptr = stagingBuffer->Map();
        memcpy(ptr, data, size);
        stagingBuffer->Unmap();

        // Flush
        KGPU::ICommandList* cmdList = CommandListRecycler::RequestCommandList();
        KGPU::BufferDesc dstDesc = buffer->GetDesc();

        KGPU::BufferBarrier dstBarrier(buffer);
        dstBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
        dstBarrier.DestStage = KGPU::PipelineStage::kCopy;
        dstBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        dstBarrier.DestAccess = KGPU::ResourceAccess::kTransferWrite;

        KGPU::BufferBarrier stagingBarrier(stagingBuffer);
        stagingBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
        stagingBarrier.DestStage = KGPU::PipelineStage::kCopy;
        stagingBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        stagingBarrier.DestAccess = KGPU::ResourceAccess::kTransferRead;

        KGPU::BarrierGroup firstGroup = {};
        firstGroup.BufferBarriers = { dstBarrier, stagingBarrier };

        KGPU::BufferBarrier dstBarrierAfter(buffer);
        dstBarrierAfter.SourceStage = KGPU::PipelineStage::kCopy;
        dstBarrierAfter.SourceAccess = KGPU::ResourceAccess::kTransferWrite;
        if (Any(dstDesc.Usage & KGPU::BufferUsage::kVertex)) {
            dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kVertexBufferRead;
            dstBarrierAfter.DestStage = KGPU::PipelineStage::kVertexInput;
        }
        if (Any(dstDesc.Usage & KGPU::BufferUsage::kIndex)) {
            dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kIndexBufferRead;
            dstBarrierAfter.DestStage = KGPU::PipelineStage::kVertexInput;
        }
        if (Any(dstDesc.Usage & KGPU::BufferUsage::kConstant)) {
            dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kConstantBufferRead;
            dstBarrierAfter.DestStage = KGPU::PipelineStage::kAllCommands;
        }
        if (Any(dstDesc.Usage & KGPU::BufferUsage::kShaderRead)) {
            dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kShaderRead;
            dstBarrierAfter.DestStage = KGPU::PipelineStage::kAllGraphics;
        }
        if (Any(dstDesc.Usage & KGPU::BufferUsage::kShaderWrite)) {
            dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kShaderWrite;
            dstBarrierAfter.DestStage = KGPU::PipelineStage::kAllGraphics;
        }

        cmdList->Barrier(firstGroup);
        cmdList->CopyBufferToBufferFull(buffer, stagingBuffer);
        cmdList->Barrier(dstBarrierAfter);
    }
}
