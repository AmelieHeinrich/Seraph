//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-21 23:27:03
//

#include "Gfx_Mipmapper.h"
#include "Gfx_ShaderManager.h"
#include "Gfx_TempResourceStorage.h"
#include "Gfx_CommandListRecycler.h"

namespace Gfx
{
    void Mipmapper::Initialize()
    {
        ShaderManager::SubscribeCompute("data/kd/shaders/mipmap.kds");
    }

    void Mipmapper::ComputeMipmaps(KGPU::ITexture* texture)
    {
        KD_INFO("Generating mipmaps for your texture. I'd ask you to kindly pack them but I didn't implement that yet :(");

        KGPU::ISampler* tempSampler = TempResourceStorage::CreateSampler(KGPU::SamplerDesc(KGPU::SamplerAddress::kClamp, KGPU::SamplerFilter::kLinear, false));
        KC::Array<KGPU::ITextureView*> ReadViews;
        KC::Array<KGPU::ITextureView*> WriteViews;
        KC::Array<KGPU::TextureBarrier> ReadBarriers;
        KC::Array<KGPU::TextureBarrier> WriteBarriers;

        // Create views for each mip level
        for (uint i = 0; i < texture->GetDesc().MipLevels; i++) {
            if (i < texture->GetDesc().MipLevels - 1) {
                // Source views are read-only
                KGPU::TextureViewDesc readViewDesc(texture, KGPU::TextureViewType::kShaderRead);
                readViewDesc.ViewMip = i;
                ReadViews.push_back(TempResourceStorage::CreateTextureView(readViewDesc));
            }
            if (i > 0) {
                // Destination views need storage access for writing
                KGPU::TextureViewDesc writeViewDesc(texture, KGPU::TextureViewType::kShaderWrite);
                writeViewDesc.ViewMip = i;
                WriteViews.push_back(TempResourceStorage::CreateTextureView(writeViewDesc));
            }
        }

        // Generate barriers
        for (uint i = 0; i < texture->GetDesc().MipLevels - 1; i++) {
            if (i > 0) {
                KGPU::TextureBarrier readBarrier(texture);
                readBarrier.BaseMipLevel = i;
                readBarrier.LevelCount = 1;
                readBarrier.SourceAccess = KGPU::ResourceAccess::kShaderWrite;
                readBarrier.DestAccess = KGPU::ResourceAccess::kShaderRead;
                readBarrier.SourceStage = KGPU::PipelineStage::kComputeShader;
                readBarrier.DestStage = KGPU::PipelineStage::kComputeShader;
                readBarrier.NewLayout = KGPU::ResourceLayout::kReadOnly;
                ReadBarriers.push_back(readBarrier);
            }

            KGPU::TextureBarrier writeBarrier(texture);
            writeBarrier.BaseMipLevel = i + 1;
            writeBarrier.LevelCount = 1;
            writeBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
            writeBarrier.DestAccess = KGPU::ResourceAccess::kShaderWrite;
            writeBarrier.SourceStage = KGPU::PipelineStage::kTopOfPipe;
            writeBarrier.DestStage = KGPU::PipelineStage::kComputeShader;
            writeBarrier.NewLayout = KGPU::ResourceLayout::kGeneral;
            WriteBarriers.push_back(writeBarrier);
        }
        
        // Dispatch shader
        KGPU::IComputePipeline* pipeline = ShaderManager::GetCompute("data/kd/shaders/mipmap.kds");
        KGPU::ICommandList* cmdList = CommandListRecycler::RequestCommandList();
        cmdList->SetComputePipeline(pipeline);

        for (int i = 0; i < texture->GetDesc().MipLevels - 1; i++) {
            uint width = std::max(1u, texture->GetDesc().Width >> (i + 1));
            uint height = std::max(1u, texture->GetDesc().Height >> (i + 1));

            struct PushConstants {
                KGPU::BindlessHandle SrcIndex;
                KGPU::BindlessHandle DstIndex;
                KGPU::BindlessHandle SamplerIndex;
                uint Pad;

                KGPU::float2 MipSize;
                KGPU::float2 Pad2;
            } constants = {
                ReadViews[i]->GetBindlessHandle(),
                WriteViews[i]->GetBindlessHandle(),  // i is correct here because WriteViews starts from mip 1
                tempSampler->GetBindlessHandle(),
                0,

                KGPU::float2(width, height),
                KGPU::float2()
            };
            
            if (i > 0) {
                cmdList->Barrier(ReadBarriers[i - 1]);
            }
            cmdList->Barrier(WriteBarriers[i]);
            cmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
            cmdList->Dispatch((width + 7) / 8, (height + 7) / 8, 1);
        }
    }
}
