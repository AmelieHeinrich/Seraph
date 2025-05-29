//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

Application::Application()
{
    mWindow = SharedPtr<Window>(new Window(1280, 720, "Seraph"));
    
    mDevice = IRHIDevice::CreateDevice(RHIBackend::kVulkan, true);
    mSurface = mDevice->CreateSurface(mWindow.get());
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
}

Application::~Application()
{
    delete mF2FSync;
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete mCommandBuffers[i];
    }
    delete mGraphicsQueue;
    delete mSurface;
    delete mDevice;
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

        commandBuffer->Barrier(beginRenderBarrier);
        commandBuffer->ClearColor(swapchainTextureView, 1.0f, 0.0f, 0.0f);
        commandBuffer->Barrier(endRenderBarrier);
        commandBuffer->End();

        mF2FSync->EndSynchronize(mCommandBuffers[frameIndex]);
        mF2FSync->PresentSurface();

        firstFrame++;
    }
}
