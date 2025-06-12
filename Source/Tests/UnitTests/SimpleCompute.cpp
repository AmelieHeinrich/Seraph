//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-06 21:19:46
//

#include "Test.h"
#include "Base.h"

class SimpleComputeTest : public RHIBaseTest
{
public:
    SimpleComputeTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kShaderWrite));

        CompiledShader shader = ShaderCompiler::Compile("Tests/SimpleCompute.slang", { "CSMain" });

        RHIComputePipelineDesc desc = {};
        desc.ComputeBytecode = shader.Entries["CSMain"];
        desc.PushConstantSize = sizeof(uint);
        mComputePipeline = mStarters.Device->CreateComputePipeline(desc);
    }

    ~SimpleComputeTest()
    {
        delete mComputePipeline;
        delete mView;
    }

    void Execute() override
    {
        RHITextureBarrier beginRenderBarrier(mStarters.RenderTexture);
        beginRenderBarrier.SourceStage = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage = RHIPipelineStage::kComputeShader;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess = RHIResourceAccess::kShaderWrite;
        beginRenderBarrier.NewLayout = RHIResourceLayout::kGeneral;

        RHITextureBarrier endRenderBarrier(mStarters.RenderTexture);
        endRenderBarrier.SourceStage = RHIPipelineStage::kComputeShader;
        endRenderBarrier.DestStage = RHIPipelineStage::kCopy;
        endRenderBarrier.SourceAccess = RHIResourceAccess::kShaderWrite;
        endRenderBarrier.DestAccess = RHIResourceAccess::kMemoryRead;
        endRenderBarrier.NewLayout = RHIResourceLayout::kTransferSrc;

        struct PushConstants {
            BindlessHandle handle;
        } handle = {
            mView->GetBindlessHandle(),
        };

        mCommandList->Barrier(beginRenderBarrier);
        mCommandList->SetComputePipeline(mComputePipeline);
        mCommandList->SetComputeConstants(mComputePipeline, &handle, sizeof(handle));
        mCommandList->Dispatch((TEST_WIDTH + 7) / 8, (TEST_HEIGHT + 7) / 8, 1);
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
    IRHIComputePipeline* mComputePipeline;
};

DEFINE_RHI_TEST(SimpleCompute) {
    SimpleComputeTest test(backend);
    return test.Run();
}
