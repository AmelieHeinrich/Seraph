//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <filesystem>

static const float VERTICES[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
};

static const uint INDICES[] = {
    0, 1, 3,
    1, 2, 3
};

Application::Application()
    : mBackend(RHIBackend::kVulkan)
{
    ShaderCompiler::Initialize(mBackend);
    CompiledShader shader = ShaderCompiler::Compile("Textured", { "VSMain", "FSMain" });

    mWindow = SharedPtr<Window>(new Window(1280, 720, "Seraph"));
    mDevice = IRHIDevice::CreateDevice(mBackend, true);
    mSurface = mDevice->CreateSurface(mWindow.get());
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
    Uploader::Initialize(mDevice, mGraphicsQueue);

    mVertexBuffer = mDevice->CreateBuffer(RHIBufferDesc(sizeof(VERTICES), sizeof(float) * 6, RHIBufferUsage::kVertex));
    mIndexBuffer = mDevice->CreateBuffer(RHIBufferDesc(sizeof(INDICES), sizeof(uint), RHIBufferUsage::kIndex));
    mSampler = mDevice->CreateSampler(RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, false));
    {
        RHITextureDesc desc = {};
        desc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
        desc.Width = 1;
        desc.Height = 1;
        desc.Depth = 1;
        desc.MipLevels = 1;
        desc.Usage = RHITextureUsage::kShaderResource;
        mTexture = mDevice->CreateTexture(desc);
        mTextureSRV = mDevice->CreateTextureView(RHITextureViewDesc(mTexture, RHITextureViewType::kShaderRead));
    }

    uint color = 0xFF00FFFF;

    Uploader::EnqueueTextureUploadRaw(&color, sizeof(uint), mTexture);
    Uploader::EnqueueBufferUpload(VERTICES, sizeof(VERTICES), mVertexBuffer);
    Uploader::EnqueueBufferUpload(INDICES, sizeof(INDICES), mIndexBuffer);
    Uploader::Flush();

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.ReflectInputLayout = true;
    desc.PushConstantSize = sizeof(BindlessHandle) * 2;
    desc.RenderTargetFormats.push_back(mSurface->GetTexture(0)->GetDesc().Format);

    mPipeline = mDevice->CreateGraphicsPipeline(desc);
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mTextureSRV;
    delete mTexture;
    delete mSampler;
    delete mIndexBuffer;
    delete mVertexBuffer;
    delete mPipeline;
    delete mF2FSync;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mCommandBuffers[i];
    }
    delete mGraphicsQueue;
    delete mSurface;
    delete mDevice;

    ShaderCompiler::Shutdown();
}

void Application::Run()
{
    int firstFrame = 0;

    while (mWindow->IsOpen()) {
        mWindow->PollEvents();

        uint frameIndex = mF2FSync->BeginSynchronize();
        IRHICommandBuffer* commandBuffer = mCommandBuffers[frameIndex];
        IRHITexture* swapchainTexture = mSurface->GetTexture(frameIndex);
        IRHITextureView* swapchainTextureView = mSurface->GetTextureView(frameIndex);

        commandBuffer->Reset();
        commandBuffer->Begin();
        
        RHITextureBarrier beginRenderBarrier(swapchainTexture);
        beginRenderBarrier.SourceStage  = RHIPipelineStage::kBottomOfPipe;
        beginRenderBarrier.DestStage    = RHIPipelineStage::kCopy;
        beginRenderBarrier.SourceAccess = RHIResourceAccess::kNone;
        beginRenderBarrier.DestAccess   = RHIResourceAccess::kTransferWrite;
        beginRenderBarrier.OldLayout    = firstFrame < 3 ? RHIResourceLayout::kUndefined : RHIResourceLayout::kPresent;
        beginRenderBarrier.NewLayout    = RHIResourceLayout::kTransferDst;

        RHITextureBarrier endRenderBarrier(swapchainTexture);
        endRenderBarrier.SourceStage   = RHIPipelineStage::kCopy;
        endRenderBarrier.DestStage     = RHIPipelineStage::kBottomOfPipe;
        endRenderBarrier.SourceAccess  = RHIResourceAccess::kTransferWrite;
        endRenderBarrier.DestAccess    = RHIResourceAccess::kNone;
        endRenderBarrier.OldLayout     = RHIResourceLayout::kTransferDst;
        endRenderBarrier.NewLayout     = RHIResourceLayout::kPresent;

        RHIRenderAttachment attachment(swapchainTextureView);
        RHIRenderBegin renderBegin(1280, 720, { RHIRenderAttachment(swapchainTextureView) }, {});

        struct PushConstant {
            BindlessHandle Texture;
            BindlessHandle Sampler;
        } constant = {
            mTextureSRV->GetBindlessHandle(),
            mSampler->GetBindlessHandle()
        };

        commandBuffer->Barrier(beginRenderBarrier);
        commandBuffer->BeginRendering(renderBegin);
        commandBuffer->SetGraphicsPipeline(mPipeline);
        commandBuffer->SetViewport(renderBegin.Width, renderBegin.Height, 0, 0);
        commandBuffer->SetVertexBuffer(mVertexBuffer);
        commandBuffer->SetIndexBuffer(mIndexBuffer);
        commandBuffer->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
        commandBuffer->DrawIndexed(6, 1, 0, 0, 0);
        commandBuffer->EndRendering();
        commandBuffer->Barrier(endRenderBarrier);
        commandBuffer->End();

        mF2FSync->EndSynchronize(mCommandBuffers[frameIndex]);
        mF2FSync->PresentSurface();

        firstFrame++;
    }
}
