//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 19:10:20
//

#include "Test.h"
#include "Base.h"

class StreamedTriangleTest : public RHIBaseTest
{
public:
    StreamedTriangleTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kRenderTarget));

        CompiledShader shader = ShaderCompiler::Compile("Tests/StreamedTriangle.slang", { "VSMain", "FSMain" });

        RHIGraphicsPipelineDesc desc = {};
        desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
        desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
        desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
        mPipeline = mStarters.Device->CreateGraphicsPipeline(desc);
    }

    ~StreamedTriangleTest()
    {
        delete mPipeline;
        delete mView;
    }

    void Execute() override
    {
        RHITextureBarrier beginRenderBarrier(mStarters.RenderTexture);
        beginRenderBarrier.SourceStage = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage = RHIPipelineStage::kColorAttachmentOutput;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess = RHIResourceAccess::kColorAttachmentWrite;
        beginRenderBarrier.NewLayout = RHIResourceLayout::kColorAttachment;

        RHITextureBarrier endRenderBarrier(mStarters.RenderTexture);
        endRenderBarrier.SourceStage = RHIPipelineStage::kColorAttachmentOutput;
        endRenderBarrier.DestStage = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kColorAttachmentWrite;
        endRenderBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout = RHIResourceLayout::kTransferSrc;

        RHIRenderAttachment attachment(mView);
        RHIRenderBegin renderBegin(TEST_WIDTH, TEST_HEIGHT, { attachment }, {});

        mCommandList->Barrier(beginRenderBarrier);
        mCommandList->BeginRendering(renderBegin);
        mCommandList->SetViewport(TEST_WIDTH, TEST_HEIGHT, 0.f, 0.f);
        mCommandList->SetGraphicsPipeline(mPipeline);
        mCommandList->Draw(3, 1, 0, 0);
        mCommandList->EndRendering();
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
    IRHIGraphicsPipeline* mPipeline;
};

DEFINE_RHI_TEST(StreamedTriangle) {
    StreamedTriangleTest test(backend);
    return test.Run();
}
