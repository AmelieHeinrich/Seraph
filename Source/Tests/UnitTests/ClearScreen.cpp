//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 18:15:18
//

#include "Test.h"
#include "Base.h"

class ClearScreenTest : public RHIBaseTest
{
public:
    ClearScreenTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kRenderTarget));
    }

    ~ClearScreenTest()
    {
        delete mView;
    }

    void Execute() override
    {
        RHITextureBarrier beginRenderBarrier(mStarters.RenderTexture);
        beginRenderBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage    = RHIPipelineStage::kColorAttachmentOutput;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess   = RHIResourceAccess::kColorAttachmentWrite;
        beginRenderBarrier.NewLayout    = RHIResourceLayout::kColorAttachment;

        RHITextureBarrier endRenderBarrier(mStarters.RenderTexture);
        endRenderBarrier.SourceStage  = RHIPipelineStage::kColorAttachmentOutput;
        endRenderBarrier.DestStage    = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kColorAttachmentWrite;
        endRenderBarrier.DestAccess   = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout    = RHIResourceLayout::kTransferSrc;

        RHIRenderAttachment attachment(mView);
        RHIRenderBegin renderBegin(mStarters.RenderTexture->GetDesc().Width, mStarters.RenderTexture->GetDesc().Height, { attachment }, {});

        mCommandList->Barrier(beginRenderBarrier);
        mCommandList->BeginRendering(renderBegin);
        mCommandList->EndRendering();
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
};

DEFINE_RHI_TEST(ClearScreen) {
    ClearScreenTest test(backend);
    return test.Run();
}
