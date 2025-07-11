//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 07:23:21
//

#include "Test.h"
#include "Base.h"

class BlockCompressedTest : public RHIBaseTest
{
public:
    BlockCompressedTest(RHIBackend backend)
        : RHIBaseTest(backend)
    {
        Compressor compressor;
        compressor.CompressTexture("Data/Textures/BCTest.jpg");

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

        mVertexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(vertices), sizeof(float3) + sizeof(float2), RHIBufferUsage::kVertex));
        mIndexBuffer = mStarters.Device->CreateBuffer(RHIBufferDesc(sizeof(indices), sizeof(uint), RHIBufferUsage::kIndex));

        std::string path = compressor.ToCachedPath("Data/Textures/BCTest.jpg");
        TextureAsset asset;
        asset.Load(path);

        RHITextureDesc checkerboardDesc = {};
        checkerboardDesc.Width = asset.Header.Width;
        checkerboardDesc.Height = asset.Header.Height;
        checkerboardDesc.MipLevels = asset.Header.Mips;
        checkerboardDesc.Format = asset.Header.Format;
        checkerboardDesc.Usage = RHITextureUsage::kShaderResource;

        mTexture = mStarters.Device->CreateTexture(checkerboardDesc);
        mTextureView = mStarters.Device->CreateTextureView(RHITextureViewDesc(mTexture, RHITextureViewType::kShaderRead));
        mSampler = mStarters.Device->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, true));

        Uploader::EnqueueTextureUploadRaw(asset.Pixels.data(), asset.Pixels.size(), mTexture);
        Uploader::EnqueueBufferUpload(vertices, sizeof(vertices), mVertexBuffer);
        Uploader::EnqueueBufferUpload(indices, sizeof(indices), mIndexBuffer);
        Uploader::Flush();

        CompiledShader shader = ShaderCompiler::Compile("Tests/TexturedDraw.hlsl");

        RHIGraphicsPipelineDesc desc = {};
        desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
        desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
        desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
        desc.ReflectInputLayout = true;
        desc.PushConstantSize = sizeof(uint) * 4;
        mPipeline = mStarters.Device->CreateGraphicsPipeline(desc);
    }

    ~BlockCompressedTest()
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

DEFINE_RHI_TEST(BlockCompressed) {
    BlockCompressedTest test(backend);
    return test.Run();
}
