//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-06 21:19:46
//

#include "Test.h"

DEFINE_RHI_TEST(SimpleCompute) {
    TestStarters starters = ITest::CreateStarters(backend);
    
    IRHITextureView* view = starters.Device->CreateTextureView(RHITextureViewDesc(starters.RenderTexture, RHITextureViewType::kShaderWrite));
    IRHICommandList* cmdBuf = starters.Queue->CreateCommandBuffer(true);
    
    CompiledShader shader = ShaderCompiler::Compile("Tests/SimpleCompute.slang", { "CSMain" });

    RHIComputePipelineDesc desc = {};
    desc.ComputeBytecode = shader.Entries["CSMain"];
    desc.PushConstantSize = sizeof(uint);
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
        } handle = {
            view->GetBindlessHandle()
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

    delete pipeline;
    delete cmdBuf;
    delete view;
    ITest::DeleteStarts(starters);

    return { std::move(starters.ScreenshotData), true };
}
