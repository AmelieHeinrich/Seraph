//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-12 22:07:56
//

#include "Base.h"

RHIBaseTest::RHIBaseTest(RHIBackend backend)
{
    mStarters = ITest::CreateStarters(backend);
}

TestResult RHIBaseTest::Run()
{
    BeginCmd();
    Execute();
    EndAndSubmitCmd();

    CopyRenderToScreenshot();
    Cleanup();
    return { std::move(mStarters.ScreenshotData), true };
}

void RHIBaseTest::BeginCmd()
{
    mCommandList = mStarters.Queue->CreateCommandBuffer(true);
    mCommandList->Begin();
}

void RHIBaseTest::EndAndSubmitCmd()
{
    mCommandList->End();
    mStarters.Queue->SubmitAndFlushCommandBuffer(mCommandList);
    delete mCommandList;
}

void RHIBaseTest::CopyRenderToScreenshot()
{
    BeginCmd();

    RHIBufferBarrier beginBufferBarrier(mStarters.ScreenshotBuffer);
    beginBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryRead;
    beginBufferBarrier.DestAccess = RHIResourceAccess::kMemoryWrite;
    beginBufferBarrier.SourceStage = RHIPipelineStage::kAllCommands;
    beginBufferBarrier.DestStage = RHIPipelineStage::kCopy;

    RHIBufferBarrier endBufferBarrier(mStarters.ScreenshotBuffer);
    endBufferBarrier.SourceAccess = RHIResourceAccess::kMemoryWrite;
    endBufferBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
    endBufferBarrier.SourceStage = RHIPipelineStage::kCopy;
    endBufferBarrier.DestStage = RHIPipelineStage::kAllCommands;

    mCommandList->Barrier(beginBufferBarrier);
    mCommandList->CopyTextureToBuffer(mStarters.ScreenshotBuffer, mStarters.RenderTexture);
    mCommandList->Barrier(endBufferBarrier);

    EndAndSubmitCmd();

    void* data = mStarters.ScreenshotBuffer->Map();
    memcpy(mStarters.ScreenshotData.Pixels.data(), data, mStarters.ScreenshotData.Pixels.size());
    mStarters.ScreenshotBuffer->Unmap();
}

void RHIBaseTest::Cleanup()
{
    ITest::DeleteStarts(mStarters);
}
