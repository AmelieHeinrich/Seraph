//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-12 22:48:14
//

#include "Test.h"
#include "Base.h"

class IndexDrawingTest : public RHIBaseTest
{
public:
    IndexDrawingTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kRenderTarget));

        float vertices[] = {
             0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
             0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f
        };
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        mVertexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(glm::vec3) * 2, RHIBufferUsage::kVertex));
        mIndexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex));

        Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), mVertexBuffer);
        Uploader::EnqueueBufferUpload(indices, sizeof(indices), mIndexBuffer);
        Uploader::Flush();

        CompiledShader shader = ShaderCompiler::Compile("Tests/IndexedDraw.slang", { "VSMain", "FSMain" });

        RHIGraphicsPipelineDesc desc = {};
        desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
        desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
        desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
        desc.ReflectInputLayout = true;
        mPipeline = mStarters.Device->CreateGraphicsPipeline(desc);
    }

    ~IndexDrawingTest()
    {
        delete mIndexBuffer;
        delete mVertexBuffer;
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
        mCommandList->SetVertexBuffer(mVertexBuffer);
        mCommandList->SetIndexBuffer(mIndexBuffer);
        mCommandList->DrawIndexed(6, 1, 0, 0, 0);
        mCommandList->EndRendering();
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
    IRHIGraphicsPipeline* mPipeline;

    IRHIBuffer* mVertexBuffer;
    IRHIBuffer* mIndexBuffer;
};

DEFINE_RHI_TEST(IndexDrawing) {
    IndexDrawingTest test(backend);
    return test.Run();
}
