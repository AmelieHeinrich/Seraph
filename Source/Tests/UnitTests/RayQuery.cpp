//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 12:00:05
//

#include "Test.h"

#include <glm/gtc/matrix_transform.hpp>

DEFINE_RHI_TEST(RayQuery) {
    TestStarters starters = ITest::CreateStarters(backend);

    uint indices[] = {
        0, 1, 2
    };

    glm::vec3 vertices[] = {
        glm::vec3{  0.0f, -0.5f, 1.0f },
        glm::vec3{ -0.5f,  0.5f, 1.0f },
        glm::vec3{  0.5f,  0.5f, 1.0f }
    };

    IRHIBuffer* vertexBuffer = starters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(glm::vec3), RHIBufferUsage::kVertex));
    IRHIBuffer* indexBuffer = starters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex));
    IRHIBLAS* blas = starters.Device->CreateBLAS(RHIBLASDesc(vertexBuffer, indexBuffer));
    IRHITLAS* tlas = starters.Device->CreateTLAS();

    TLASInstance instance = {};
    instance.Transform = glm::identity<glm::mat3x4>();
    instance.AccelerationStructureReference = blas->GetAddress();
    instance.Mask = 1;
    instance.InstanceCustomIndex = 0;
    instance.Flags = TLAS_INSTANCE_OPAQUE;

    IRHIBuffer* instanceBuffer = starters.Device->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance), sizeof(TLASInstance), RHIBufferUsage::kConstant));
    void* ptr = instanceBuffer->Map();
    memcpy(ptr, &instance, sizeof(instance));
    instanceBuffer->Unmap();

    Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), vertexBuffer);
    Uploader::EnqueueBufferUpload(indices, sizeof(indices), indexBuffer);
    Uploader::EnqueueBLASBuild(blas);
    Uploader::EnqueueTLASBuild(tlas, instanceBuffer, 1);
    Uploader::Flush();
    
    IRHITextureView* view = starters.Device->CreateTextureView(RHITextureViewDesc(starters.RenderTexture, RHITextureViewType::kShaderWrite));
    IRHICommandBuffer* cmdBuf = starters.Queue->CreateCommandBuffer(true);
    
    CompiledShader shader = ShaderCompiler::Compile("Tests/RayQuery.slang", { "CSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint) * 2;
    IRHIComputePipeline* pipeline = starters.Device->CreateComputePipeline(desc);

    cmdBuf->Begin();
    {
        RHITextureBarrier beginRenderBarrier(starters.RenderTexture);
        beginRenderBarrier.SourceStage = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage = RHIPipelineStage::kComputeShader;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess = RHIResourceAccess::kShaderWrite;
        beginRenderBarrier.NewLayout = RHIResourceLayout::kGeneral;

        RHITextureBarrier endRenderBarrier(starters.RenderTexture);
        endRenderBarrier.SourceStage = RHIPipelineStage::kComputeShader;
        endRenderBarrier.DestStage = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kShaderWrite;
        endRenderBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout = RHIResourceLayout::kTransferSrc;

        struct PushConstants {
            BindlessHandle handle;
            BindlessHandle tlas;
        } handle = {
            view->GetBindlessHandle(),
            tlas->GetBindlessHandle()
        };

        cmdBuf->Barrier(beginRenderBarrier);
        cmdBuf->SetComputePipeline(pipeline);
        cmdBuf->SetComputeConstants(pipeline, &handle, sizeof(handle));
        cmdBuf->Dispatch((TEST_WIDTH + 7) / 8, (TEST_HEIGHT + 7) / 8, 1);
        cmdBuf->Barrier(endRenderBarrier);
    }
    cmdBuf->End();
    starters.Queue->SubmitAndFlushCommandBuffer(cmdBuf);
    delete cmdBuf;

    cmdBuf = starters.Queue->CreateCommandBuffer(true);
    cmdBuf->Begin();
    {
        RHIBufferBarrier beginBufferBarrier(starters.ScreenshotBuffer);
        beginBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryRead;
        beginBufferBarrier.DestAccess = RHIResourceAccess::kMemoryWrite;
        beginBufferBarrier.SourceStage = RHIPipelineStage::kAllCommands;
        beginBufferBarrier.DestStage = RHIPipelineStage::kCopy;

        RHIBufferBarrier endBufferBarrier(starters.ScreenshotBuffer);
        endBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryWrite;
        endBufferBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endBufferBarrier.SourceStage = RHIPipelineStage::kCopy;
        endBufferBarrier.DestStage = RHIPipelineStage::kAllCommands;

        cmdBuf->Barrier(beginBufferBarrier);
        cmdBuf->CopyTextureToBuffer(starters.ScreenshotBuffer, starters.RenderTexture);
        cmdBuf->Barrier(endBufferBarrier);
    }
    cmdBuf->End();
    starters.Queue->SubmitAndFlushCommandBuffer(cmdBuf);

    void* data = starters.ScreenshotBuffer->Map();
    memcpy(starters.ScreenshotData.Pixels.data(), data, starters.ScreenshotData.Pixels.size());
    starters.ScreenshotBuffer->Unmap();

    delete tlas;
    delete blas;
    delete indexBuffer;
    delete vertexBuffer;
    delete pipeline;
    delete cmdBuf;
    delete view;
    ITest::DeleteStarts(starters);

    return { std::move(starters.ScreenshotData), true };
}
