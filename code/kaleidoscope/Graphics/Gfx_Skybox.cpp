//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-20 15:14:40
//

#include "Gfx_Skybox.h"
#include "Gfx_ShaderManager.h"
#include "Gfx_Uploader.h"
#include "Gfx_TempResourceStorage.h"
#include "Gfx_CommandListRecycler.h"

#include <KDAsset/KDA_TextureLoader.h>

namespace Gfx
{
    void SkyboxCooker::Initialize()
    {
        ShaderManager::SubscribeCompute("data/kd/shaders/skybox_cook.kds");
    }

    void SkyboxCooker::GenerateSkybox(Skybox& sky, const KC::String& path)
    {
        // Load image
        KDA::Texture texture = KDA::TextureLoader::LoadFromFile(path);
        KGPU::ITexture* inputTex = TempResourceStorage::CreateTexture(texture.ToTextureDesc());
        KGPU::ITextureView* inputView = TempResourceStorage::CreateTextureView(KGPU::TextureViewDesc(inputTex, KGPU::TextureViewType::kShaderRead));
        KGPU::ISampler* defaultSampler = TempResourceStorage::CreateSampler(KGPU::SamplerDesc(KGPU::SamplerAddress::kWrap, KGPU::SamplerFilter::kLinear, false));
        Uploader::EnqueueTextureUploadRaw(texture.Bytes.data(), texture.Bytes.size(), inputTex);

        // NOT TEMPORARY
        KGPU::TextureDesc outDesc;
        outDesc.Width = 2048;
        outDesc.Height = 2048;
        outDesc.Depth = 6;
        outDesc.Format = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        outDesc.Usage = KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;
        outDesc.MipLevels = 1;
        sky.EnvironmentMap = Manager::GetDevice()->CreateTexture(outDesc);

        KGPU::TextureViewDesc cubeViewDesc;
        cubeViewDesc.Texture = sky.EnvironmentMap;
        cubeViewDesc.ArrayLayer = KGPU::VIEW_ALL_MIPS;
        cubeViewDesc.Dimension = KGPU::TextureViewDimension::kTextureCube;
        cubeViewDesc.ViewFormat = KGPU::TextureFormat::kR16G16B16A16_FLOAT;
        cubeViewDesc.Type = KGPU::TextureViewType::kShaderRead;
        cubeViewDesc.ViewMip = KGPU::VIEW_ALL_MIPS;
        sky.CubeView = Manager::GetDevice()->CreateTextureView(cubeViewDesc);
        //

        cubeViewDesc.Type = KGPU::TextureViewType::kShaderWrite;
        auto cubeWrite = TempResourceStorage::CreateTextureView(cubeViewDesc);

        // Run shader
        KGPU::IComputePipeline* pipeline = ShaderManager::GetCompute("data/kd/shaders/skybox_cook.kds");

        struct Constants {
            KGPU::BindlessHandle HDRInput;
            KGPU::BindlessHandle CubemapOutput;
            KGPU::BindlessHandle CubeSampler;
            uint Pad;

            uint Width;
            uint Height;
            KGPU::float2 Pad1;
        } constants = {
            inputView->GetBindlessHandle(),
            cubeWrite->GetBindlessHandle(),
            defaultSampler->GetBindlessHandle(),
            0,

            2048,
            2048,
            {}
        };

        KGPU::TextureBarrier beginBarrier(sky.EnvironmentMap);
        beginBarrier.ArrayLayer = 0;
        beginBarrier.LayerCount = 6;
        beginBarrier.BaseMipLevel = 0;
        beginBarrier.LevelCount = 1;
        beginBarrier.NewLayout = KGPU::ResourceLayout::kGeneral;
        beginBarrier.SourceStage = KGPU::PipelineStage::kNone;
        beginBarrier.DestStage = KGPU::PipelineStage::kComputeShader;
        beginBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
        beginBarrier.DestAccess = KGPU::ResourceAccess::kShaderWrite;

        KGPU::TextureBarrier endBarrier(sky.EnvironmentMap);
        endBarrier.ArrayLayer = 0;
        endBarrier.LayerCount = 6;
        endBarrier.BaseMipLevel = 0;
        endBarrier.LevelCount = 1;
        endBarrier.NewLayout = KGPU::ResourceLayout::kReadOnly;
        endBarrier.SourceStage = KGPU::PipelineStage::kComputeShader;
        endBarrier.DestStage = KGPU::PipelineStage::kPixelShader;
        endBarrier.SourceAccess = KGPU::ResourceAccess::kShaderWrite;
        endBarrier.DestAccess = KGPU::ResourceAccess::kShaderRead;

        auto cmdList = CommandListRecycler::RequestCommandList();
        cmdList->Barrier(beginBarrier);
        cmdList->SetComputePipeline(pipeline);
        cmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        cmdList->Dispatch(2048 / 32, 2048 / 32, 6);
        cmdList->Barrier(endBarrier);
    }
}
