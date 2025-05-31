//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <filesystem>

static const float VERTICES[] = {
     0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f
};

static const uint INDICES[] = {
    0, 1, 3,
    1, 2, 3
};

Application::Application()
    : mBackend(RHIBackend::kVulkan)
{
    ShaderCompiler::Initialize(mBackend);
    CompiledShader shader = ShaderCompiler::Compile("StreamedTriangle", { "VSMain", "FSMain" });

    mWindow = SharedPtr<Window>(new Window(1280, 720, "Seraph"));
    mDevice = IRHIDevice::CreateDevice(mBackend, true);
    mSurface = mDevice->CreateSurface(mWindow.get());
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
    Uploader::Initialize(mDevice, mGraphicsQueue);

    {
        RHIBufferDesc bufferDesc = {};
        bufferDesc.Size = sizeof(VERTICES);
        bufferDesc.Stride = sizeof(float) * 6;
        bufferDesc.Usage = RHIBufferUsage::kVertex;
        mVertexBuffer = mDevice->CreateBuffer(bufferDesc);
    }
    {
        RHIBufferDesc bufferDesc = {};
        bufferDesc.Size = sizeof(INDICES);
        bufferDesc.Stride = sizeof(uint);
        bufferDesc.Usage = RHIBufferUsage::kIndex;
        mIndexBuffer = mDevice->CreateBuffer(bufferDesc);
    }

    Uploader::EnqueueBufferUpload(VERTICES, sizeof(VERTICES), mVertexBuffer);
    Uploader::EnqueueBufferUpload(INDICES, sizeof(INDICES), mIndexBuffer);
    Uploader::Flush();

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.ReflectInputLayout = true;
    desc.RenderTargetFormats.push_back(mSurface->GetTexture(0)->GetDesc().Format);

    mPipeline = mDevice->CreateGraphicsPipeline(desc);
}

Application::~Application()
{
    Uploader::Shutdown();

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
        
        RHITextureBarrier beginRenderBarrier = {
            .SourceStage        = RHIPipelineStage::kBottomOfPipe,
            .DestStage          = RHIPipelineStage::kCopy,
            .SourceAccess       = RHIResourceAccess::kNone,
            .DestAccess         = RHIResourceAccess::kTransferWrite,
            .OldLayout          = firstFrame < 3 ? RHIResourceLayout::kUndefined : RHIResourceLayout::kPresent,
            .NewLayout          = RHIResourceLayout::kTransferDst,
            .Texture            = swapchainTexture,
            .BaseMipLevel = 0,
            .LevelCount = 1,
            .ArrayLayer = 0,
            .LayerCount = 1,
        };
        RHITextureBarrier endRenderBarrier = {
            .SourceStage        = RHIPipelineStage::kCopy,
            .DestStage          = RHIPipelineStage::kBottomOfPipe,
            .SourceAccess       = RHIResourceAccess::kTransferWrite,
            .DestAccess         = RHIResourceAccess::kNone,
            .OldLayout          = RHIResourceLayout::kTransferDst,
            .NewLayout          = RHIResourceLayout::kPresent,
            .Texture            = swapchainTexture,
            .BaseMipLevel = 0,
            .LevelCount = 1,
            .ArrayLayer = 0,
            .LayerCount = 1
        };

        RHIRenderAttachment attachment = {};
        attachment.Clear = true;
        attachment.View = swapchainTextureView;

        RHIRenderBegin renderBegin = {};
        renderBegin.Width = 1280;
        renderBegin.Height = 720;
        renderBegin.RenderTargets = { attachment };
        renderBegin.DepthTarget = {};

        commandBuffer->Barrier(beginRenderBarrier);
        commandBuffer->BeginRendering(renderBegin);
        commandBuffer->SetGraphicsPipeline(mPipeline);
        commandBuffer->SetViewport(renderBegin.Width, renderBegin.Height, 0, 0);
        commandBuffer->SetVertexBuffer(mVertexBuffer);
        commandBuffer->SetIndexBuffer(mIndexBuffer);
        commandBuffer->DrawIndexed(6, 1, 0, 0, 0);
        commandBuffer->EndRendering();
        commandBuffer->Barrier(endRenderBarrier);
        commandBuffer->End();

        mF2FSync->EndSynchronize(mCommandBuffers[frameIndex]);
        mF2FSync->PresentSurface();

        firstFrame++;
    }
}
