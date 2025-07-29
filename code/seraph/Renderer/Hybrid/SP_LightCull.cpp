//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:42:51
//

#include "SP_LightCull.h"
#include "SP_GBuffer.h"
#include "SP_Application.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ViewRecycler.h>

namespace SP
{
    LightCulling::LightCulling()
    {
        // Textures
        int width, height;
        Application::Get().GetWindow()->GetSize(width, height);

        // Get sizes
        mNumTilesX = (width + TILE_WIDTH - 1) / TILE_WIDTH;
        mNumTilesY = (height + TILE_HEIGHT - 1) / TILE_HEIGHT;

        // Create resources
        Gfx::ResourceManager::CreateBuffer(LIGHT_CULL_TILE_BUFFER, KGPU::BufferDesc(
            mNumTilesX * mNumTilesY * sizeof(LightTile),
            sizeof(LightTile),
            KGPU::BufferUsage::kShaderRead | KGPU::BufferUsage::kShaderWrite
        ));
        Gfx::ResourceManager::CreateBuffer(LIGHT_CULL_TILE_INDICES_BUFFER, KGPU::BufferDesc(
            mNumTilesX * mNumTilesY * sizeof(uint) * MAX_LIGHT_PER_TILE,
            sizeof(uint),
            KGPU::BufferUsage::kShaderRead | KGPU::BufferUsage::kShaderWrite
        ));

        // Create pipeline
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/generate_tiles.kds");
        Gfx::ShaderManager::SubscribeCompute("data/sp/shaders/cull_tiles.kds");
    }

    LightCulling::~LightCulling()
    {
    }

    void LightCulling::Render(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::LightCulling::Render");
        GenerateTiles(begin);
        CullTiles(begin);
    }

    void LightCulling::GenerateTiles(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::LightCulling::GenerateTiles");
        Gfx::Resource& tileBuffer = Gfx::ResourceManager::Import(LIGHT_CULL_TILE_BUFFER, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& depthBuffer = Gfx::ResourceManager::Import(GBUFFER_DEPTH_ID, begin.CmdList, Gfx::ImportType::kShaderRead);
    
        struct PushConstants {
            BindlessHandle TileArray;
            uint TileWidth;
            uint TileHeight;
            BindlessHandle DepthMap;

            uint NumTilesX;
            uint NumTilesY;
            int Width;
            int Height;
        } constants = {
            Gfx::ViewRecycler::GetUAV(tileBuffer.Buffer)->GetBindlessHandle(),
            TILE_WIDTH,
            TILE_HEIGHT,
            Gfx::ViewRecycler::GetTextureView(KGPU::TextureViewDesc(depthBuffer.Texture, KGPU::TextureViewType::kShaderRead, KGPU::TextureFormat::kR32_FLOAT))->GetBindlessHandle(),

            mNumTilesX,
            mNumTilesY,
            begin.Width,
            begin.Height
        };

        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/generate_tiles.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch(mNumTilesX, mNumTilesY, 1);

        // Insert manual UAV barrier
        KGPU::MemoryBarrier barrier(KGPU::ResourceAccess::kShaderWrite, KGPU::ResourceAccess::kShaderWrite, KGPU::PipelineStage::kComputeShader, KGPU::PipelineStage::kComputeShader);
        begin.CmdList->Barrier(barrier);
    }

    void LightCulling::CullTiles(RenderPassBegin& begin)
    {
        KGPU::ScopedMarker _(begin.CmdList, "SP::LightCulling::CullTiles");
        Gfx::Resource& tileBuffer = Gfx::ResourceManager::Import(LIGHT_CULL_TILE_BUFFER, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& tileIndicesBuffer = Gfx::ResourceManager::Import(LIGHT_CULL_TILE_INDICES_BUFFER, begin.CmdList, Gfx::ImportType::kShaderWrite);
        Gfx::Resource& cameraBuffer = Gfx::ResourceManager::Get(GBUFFER_CAMERA_CBV_ID);

        struct PushConstants {
            BindlessHandle LightIndex;
            BindlessHandle CameraIndex;
            BindlessHandle TileArray;
            BindlessHandle BinsArray;
        
            uint TileWidth;
            uint TileHeight;
            uint NumTilesX;
            uint NumTilesY;
        
            int Width;
            int Height;
            uint PointLightCount;
            uint SpotLightCount;

            BindlessHandle SpotLightArray;
            uint3 Pad;
        } constants = {
            begin.World->GetLightList()->GetPointLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(tileBuffer.Buffer)->GetBindlessHandle(),
            Gfx::ViewRecycler::GetUAV(tileIndicesBuffer.Buffer)->GetBindlessHandle(),

            TILE_WIDTH,
            TILE_HEIGHT,
            mNumTilesX,
            mNumTilesY,

            begin.Width,
            begin.Height,
            static_cast<uint>(begin.World->GetLightList()->PointLights.size()),
            static_cast<uint>(begin.World->GetLightList()->SpotLights.size()),

            begin.World->GetLightList()->GetSpotLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            {}
        };

        KGPU::IComputePipeline* pipeline = Gfx::ShaderManager::GetCompute("data/sp/shaders/cull_tiles.kds");
        begin.CmdList->SetComputePipeline(pipeline);
        begin.CmdList->SetComputeConstants(pipeline, &constants, sizeof(constants));
        begin.CmdList->Dispatch(mNumTilesX, mNumTilesY, 1);
    }
}
