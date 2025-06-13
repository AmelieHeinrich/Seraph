//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:23:21
//

#include "Test.h"
#include "Base.h"

class TexturedDrawTest : public RHIBaseTest
{
public:
    TexturedDrawTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        mView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mStarters.RenderTexture, RHITextureViewType::kRenderTarget));

        float vertices[] = {
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        };
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        mVertexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(glm::vec3) + sizeof(glm::vec2), RHIBufferUsage::kVertex));
        mIndexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex));

        RHITextureDesc checkerboardDesc = {};
        checkerboardDesc.Width = 256;
        checkerboardDesc.Height = 256;
        checkerboardDesc.MipLevels = 1;
        checkerboardDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
        checkerboardDesc.Usage = RHITextureUsage::kShaderResource;

        mTexture = mStarters.Device->CreateTexture(checkerboardDesc);
        mTextureView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mTexture, RHITextureViewType::kShaderRead));
        mSampler = mStarters.Device->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, false));

        Array<uint> textureMemory;
        const uint32_t squareSize = 32;
        for (uint32_t y = 0; y < 256; ++y) {
            for (uint32_t x = 0; x < 256; ++x) {
                // Determine which color to use based on checkerboard pattern
                bool isWhite = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            
                textureMemory.push_back(isWhite ? 0xFFFFFFFF : 0xFF000000);
            }
        }

        Uploader::EnqueueTextureUploadRaw(textureMemory.data(), textureMemory.size(), mTexture);
        Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), mVertexBuffer);
        Uploader::EnqueueBufferUpload(indices, sizeof(indices), mIndexBuffer);
        Uploader::Flush();

        CompiledShader shader = ShaderCompiler::Compile("Tests/TexturedDraw.slang", { "VSMain", "FSMain" });

        RHIGraphicsPipelineDesc desc = {};
        desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
        desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
        desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
        desc.ReflectInputLayout = true;
        desc.PushConstantSize = sizeof(uint) * 4;
        mPipeline = mStarters.Device->CreateGraphicsPipeline(desc);
    }

    ~TexturedDrawTest()
    {
        delete mSampler;
        delete mTextureView;
        delete mTexture;
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

        struct PushConstants {
            BindlessHandle srv;
            BindlessHandle sampler;
            uint pad[2];
        } constants = {
            mTextureView->GetBindlessHandle(),
            mSampler->GetBindlessHandle()
        };

        mCommandList->Barrier(beginRenderBarrier);
        mCommandList->BeginRendering(renderBegin);
        mCommandList->SetViewport(TEST_WIDTH, TEST_HEIGHT, 0.f, 0.f);
        mCommandList->SetGraphicsPipeline(mPipeline);
        mCommandList->SetVertexBuffer(mVertexBuffer);
        mCommandList->SetIndexBuffer(mIndexBuffer);
        mCommandList->SetGraphicsConstants(mPipeline, &constants, sizeof(constants));
        mCommandList->DrawIndexed(6, 1, 0, 0, 0);
        mCommandList->EndRendering();
        mCommandList->Barrier(endRenderBarrier);
    }
private:
    IRHITextureView* mView;
    IRHIGraphicsPipeline* mPipeline;

    IRHIBuffer* mVertexBuffer;
    IRHIBuffer* mIndexBuffer;

    IRHITexture* mTexture;
    IRHITextureView* mTextureView;
    IRHISampler* mSampler;
};

DEFINE_RHI_TEST(TexturedDraw) {
    TexturedDrawTest test(backend);
    return test.Run();
}
