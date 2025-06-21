//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:56:07
//

#include "LightCulling.h"
#include "Debug.h"
#include "GBuffer.h"

#include <DemoApp/Camera.h>
#include <ImGui/imgui.h>

LightCulling::LightCulling(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Get sizes
    mNumClustersX = (width + CLUSTER_WIDTH - 1) / CLUSTER_WIDTH;
    mNumClustersY = (height + CLUSTER_HEIGHT - 1) / CLUSTER_HEIGHT;
    mNumClustersZ = CLUSTER_DEPTH;

    // Create resources
    uint clusterCount = mNumClustersX * mNumClustersY * mNumClustersZ;
    RendererResourceManager::CreateBuffer(LIGHT_CULL_CLUSTER_BUFFER, RHIBufferDesc(
        clusterCount * sizeof(LightCluster),
        sizeof(LightCluster),
        RHIBufferUsage::kShaderRead | RHIBufferUsage::kShaderWrite
    ));
    RendererResourceManager::CreateBuffer(LIGHT_CULL_CLUSTER_INDICES_BUFFER, RHIBufferDesc(
        clusterCount * sizeof(uint) * MAX_LIGHTS_PER_CLUSTER,
        sizeof(uint),
        RHIBufferUsage::kShaderRead | RHIBufferUsage::kShaderWrite
    ));

    // Create pipeline
    CODE_BLOCK("Create cull shader") {
        CompiledShader shader = ShaderCompiler::Compile("CullClusters", { "CSMain" });

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
    begin.CommandList->PushMarker("Clustered Light Culling");
    CullClusters(begin);
    begin.CommandList->PopMarker();
}

void LightCulling::CullClusters(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Cull Lights");
    CODE_BLOCK("Execute") {
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        void* ptr = cameraBuffer.RingBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, &begin.CamData, sizeof(begin.CamData));
        cameraBuffer.RingBuffer[begin.FrameIndex]->Unmap();

        RendererResource& tileBuffer = RendererResourceManager::Import(LIGHT_CULL_CLUSTER_BUFFER, begin.CommandList, RendererImportType::kShaderWrite);
        RendererResource& tileIndicesBuffer = RendererResourceManager::Import(LIGHT_CULL_CLUSTER_INDICES_BUFFER, begin.CommandList, RendererImportType::kShaderWrite);
    
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
            uint NumClustersZ;
            uint2 Pad;
        } constants = {
            begin.RenderScene->GetLights().GetPointLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            cameraBuffer.RingBufferViews[begin.FrameIndex]->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(tileBuffer.Buffer)->GetBindlessHandle(),
            RendererViewRecycler::GetUAV(tileIndicesBuffer.Buffer)->GetBindlessHandle(),

            CLUSTER_WIDTH,
            CLUSTER_HEIGHT,
            mNumClustersX,
            mNumClustersY,

            mWidth,
            mHeight,
            static_cast<uint>(begin.RenderScene->GetLights().PointLights.size()),
            static_cast<uint>(begin.RenderScene->GetLights().SpotLights.size()),

            begin.RenderScene->GetLights().GetSpotLightBufferView(begin.FrameIndex)->GetBindlessHandle(),
            mNumClustersZ,
            {}
        };
            
        begin.CommandList->SetComputePipeline(mCullPipeline);
        begin.CommandList->SetComputeConstants(mCullPipeline, &constants, sizeof(constants));
        begin.CommandList->Dispatch(mNumClustersX, mNumClustersY, mNumClustersZ);
    }
    begin.CommandList->PopMarker();
}

void LightCulling::UI(RenderPassBegin& begin)
{
}
