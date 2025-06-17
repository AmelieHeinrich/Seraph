//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:56:07
//

#include "LightCulling.h"
#include "Debug.h"
#include "GBuffer.h"

#include <ImGui/imgui.h>

LightCulling::LightCulling(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Get sizes
    mNumTilesX = (width + TILE_WIDTH - 1) / TILE_WIDTH;
    mNumTilesY = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;

    // Create resources
    RendererResourceManager::CreateBuffer(LIGHT_CULL_TILE_BUFFER, RHIBufferDesc(
        mNumTilesX * mNumTilesY * sizeof(LightTile),
        sizeof(LightTile),
        RHIBufferUsage::kShaderRead | RHIBufferUsage::kShaderWrite
    ));
    RendererResourceManager::CreateBuffer(LIGHT_CULL_TILE_INDICES_BUFFER, RHIBufferDesc(
        mNumTilesX * mNumTilesY * sizeof(uint) * MAX_LIGHT_PER_TILE,
        sizeof(uint),
        RHIBufferUsage::kShaderRead | RHIBufferUsage::kShaderWrite
    ));

    // Create pipeline
    CODE_BLOCK("Create generate shader") {
        CompiledShader shader = ShaderCompiler::Compile("GenerateTiles", { "CSMain" });

        RHIComputePipelineDesc desc = {};
        desc.ComputeBytecode = shader.Entries["CSMain"];
        desc.PushConstantSize = sizeof(uint) * 8;
        mGeneratePipeline = mParentDevice->CreateComputePipeline(desc);
    }

    CODE_BLOCK("Create generate shader") {
        CompiledShader shader = ShaderCompiler::Compile("CullTiles", { "CSMain" });

        RHIComputePipelineDesc desc = {};
        desc.ComputeBytecode = shader.Entries["CSMain"];
        desc.PushConstantSize = sizeof(uint) * 16;
        mCullPipeline = mParentDevice->CreateComputePipeline(desc);
    }
}

LightCulling::~LightCulling()
{
    delete mCullPipeline;
}

void LightCulling::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Tiled Light Culling");
    GenerateTiles(begin);
    CullTiles(begin);
    begin.CommandList->PopMarker();
}

void LightCulling::GenerateTiles(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Generate Tiles");
    {
        // Pass
        RendererResource& tileBuffer = RendererResourceManager::Import(LIGHT_CULL_TILE_BUFFER, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& depthBuffer = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kShaderRead);
    
        struct PushConstants {
            BindlessHandle TileArray;
            uint TileWidth;
            uint TileHeight;
            BindlessHandle DepthMap;

            uint NumTilesX;
            uint NumTilesY;
            uint Width;
            uint Height;
        } constants = {
            RendererViewRecycler::GetUAV(tileBuffer.Buffer)->GetBindlessHandle(),
            TILE_WIDTH,
            TILE_HEIGHT,
            RendererViewRecycler::GetTextureView(RHITextureViewDesc(depthBuffer.Texture, RHITextureViewType::kShaderRead, RHITextureFormat::kR32_FLOAT))->GetBindlessHandle(),

            mNumTilesX,
            mNumTilesY,
            mWidth,
            mHeight
        };

        begin.CommandList->SetComputePipeline(mGeneratePipeline);
        begin.CommandList->SetComputeConstants(mGeneratePipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch(mNumTilesX, mNumTilesY, 1);

        // Insert manual UAV barrier
        RHIMemoryBarrier barrier(RHIResourceAccess::kShaderWrite, RHIResourceAccess::kShaderWrite, RHIPipelineStage::kComputeShader, RHIPipelineStage::kComputeShader);
        begin.CommandList->Barrier(barrier);
    }
    begin.CommandList->PopMarker();
}

void LightCulling::CullTiles(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Cull Lights");
    {
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        void* ptr = cameraBuffer.RingBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, &begin.CamData, sizeof(begin.CamData));
        cameraBuffer.RingBuffer[begin.FrameIndex]->Unmap();

        RendererResource& tileBuffer = RendererResourceManager::Import(LIGHT_CULL_TILE_BUFFER, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& tileIndicesBuffer = RendererResourceManager::Import(LIGHT_CULL_TILE_INDICES_BUFFER, begin.CommandList, RendererImportType::kShaderWrite);
    
        struct PushConstants {
            BindlessHandle LightIndex;
            BindlessHandle CameraIndex;
            BindlessHandle TileArray;
            BindlessHandle BinsArray;
        
            uint TileWidth;
            uint TileHeight;
            uint NumTilesX;
            uint NumTilesY;
        
            uint Width;
            uint Height;
            uint PointLightCount;
            uint SpotLightCount;

            BindlessHandle SpotLightArray;
            uint3 Pad;
        } constants = {
            begin.RenderScene->GetLights().GetPointLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(tileBuffer.Buffer)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(tileIndicesBuffer.Buffer)->GetBindlessHandle(),

            TILE_WIDTH,
            TILE_HEIGHT,
            mNumTilesX,
            mNumTilesY,

            mWidth,
            mHeight,
            static_cast<uint>(begin.RenderScene->GetLights().PointLights.size()),
            static_cast<uint>(begin.RenderScene->GetLights().SpotLights.size()),

            begin.RenderScene->GetLights().GetSpotLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            {}
        };
            
        begin.CommandList->SetComputePipeline(mCullPipeline);
        begin.CommandList->SetComputeConstants(mCullPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch(mNumTilesX, mNumTilesY, 1);
    }
    begin.CommandList->PopMarker();
}

void LightCulling::UI(RenderPassBegin& begin)
{
}
