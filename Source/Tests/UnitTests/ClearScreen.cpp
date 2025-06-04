//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 18:15:18
//

#include "Test.h"

DEFINE_RHI_TEST(ClearScreen) {
    TestStarters starters = ITest::CreateStarters(backend);
    
    IRHITextureView* view = starters.Device->CreateTextureView(RHITextureViewDesc(starters.RenderTexture, RHITextureViewType::kRenderTarget));
    IRHICommandBuffer* cmdBuf = starters.Queue->CreateCommandBuffer(true);

    cmdBuf->Begin();
    {
        RHITextureBarrier beginRenderBarrier(starters.RenderTexture);
        beginRenderBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage    = RHIPipelineStage::kColorAttachmentOutput;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess   = RHIResourceAccess::kColorAttachmentWrite;
        beginRenderBarrier.NewLayout    = RHIResourceLayout::kColorAttachment;

        RHITextureBarrier endRenderBarrier(starters.RenderTexture);
        endRenderBarrier.SourceStage  = RHIPipelineStage::kColorAttachmentOutput;
        endRenderBarrier.DestStage    = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kColorAttachmentWrite;
        endRenderBarrier.DestAccess   = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout    = RHIResourceLayout::kTransferSrc;

        RHIRenderAttachment attachment(view);
        RHIRenderBegin renderBegin(starters.RenderTexture->GetDesc().Width, starters.RenderTexture->GetDesc().Height, { attachment }, {});

        cmdBuf->Barrier(beginRenderBarrier);
        cmdBuf->BeginRendering(renderBegin);
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

    delete cmdBuf;
    delete view;
    ITest::DeleteStarts(starters);

    return { std::move(starters.ScreenshotData), true };
}
