//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 19:10:20
//

#include "Test.h"

DEFINE_RHI_TEST(StreamedTriangle) {
    TestStarters starters = ITest::CreateStarters(backend);
    
    IRHITextureView* view = starters.Device->CreateTextureView(RHITextureViewDesc(starters.RenderTexture, RHITextureViewType::kRenderTarget));
    IRHICommandList* cmdBuf = starters.Queue->CreateCommandBuffer(true);
    
    CompiledShader shader = ShaderCompiler::Compile("Tests/StreamedTriangle.slang", { "VSMain", "FSMain" });

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
    IRHIGraphicsPipeline* pipeline = starters.Device->CreateGraphicsPipeline(desc);

    cmdBuf->Begin();
    {
        RHITextureBarrier beginRenderBarrier(starters.RenderTexture);
        beginRenderBarrier.SourceStage = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage = RHIPipelineStage::kColorAttachmentOutput;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess = RHIResourceAccess::kColorAttachmentWrite;
        beginRenderBarrier.NewLayout = RHIResourceLayout::kColorAttachment;

        RHITextureBarrier endRenderBarrier(starters.RenderTexture);
        endRenderBarrier.SourceStage = RHIPipelineStage::kColorAttachmentOutput;
        endRenderBarrier.DestStage = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kColorAttachmentWrite;
        endRenderBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout = RHIResourceLayout::kTransferSrc;

        RHIRenderAttachment attachment(view);
        RHIRenderBegin renderBegin(starters.RenderTexture->GetDesc().Width, starters.RenderTexture->GetDesc().Height, { attachment }, {});

        cmdBuf->Barrier(beginRenderBarrier);
        cmdBuf->BeginRendering(renderBegin);
        cmdBuf->SetViewport(TEST_WIDTH, TEST_HEIGHT, 0.f, 0.f);
        cmdBuf->SetGraphicsPipeline(pipeline);
        cmdBuf->Draw(3, 1, 0, 0);
        cmdBuf->EndRendering();
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
