//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <imgui/imgui.h>
#include <chrono>

static const float VERTICES[] = {
     0.5f,  0.5f, 0.0f, 1.0f, -1.0f,
     0.5f, -0.5f, 0.0f, 1.0f,  0.0f,
    -0.5f, -0.5f, 0.0f, 0.0f,  0.0f,
    -0.5f,  0.5f, 0.0f, 0.0f, -1.0f
};

static const uint INDICES[] = {
    0, 1, 3,
    1, 2, 3
};

Application::Application(const ApplicationSpecs& specs)
    : mSpecs(specs)
{
    ShaderCompiler::Initialize(specs.Backend);

    mWindow = SharedPtr<Window>(new Window(specs.WindowWidth, specs.WindowHeight, "Seraph"));
    mDevice = IRHIDevice::CreateDevice(specs.Backend, true);
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    mSurface = mDevice->CreateSurface(mWindow.get(), mGraphicsQueue);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
    Uploader::Initialize(mDevice, mGraphicsQueue);
    mImGuiContext = mDevice->CreateImGuiContext(mGraphicsQueue, mWindow.get());

    mScreenshotBuffer = mDevice->CreateBuffer(RHIBufferDesc(specs.WindowWidth * specs.WindowHeight * 4, 0, RHIBufferUsage::kReadback));
    mScreenshotData.Width = mSpecs.WindowWidth;
    mScreenshotData.Height = mSpecs.WindowHeight;
    mScreenshotData.Pixels.resize(mScreenshotData.Width * mScreenshotData.Height * 4);

    mRenderer = new Renderer(mDevice, mSpecs.WindowWidth, mSpecs.WindowHeight);
    
    Uploader::Flush();
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mRenderer;
    delete mScreenshotBuffer;
    delete mImGuiContext;
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
    auto lastFrame = std::chrono::high_resolution_clock::now();

    while (mWindow->IsOpen()) {
        // Start Frame
        mWindow->PollEvents();
        mCamera.Begin();

        // Calculate DT
        auto time = std::chrono::high_resolution_clock::now();
        float delta = (std::chrono::duration<float>(lastFrame - time).count());
        lastFrame = time;

        // Begin fill info
        RenderPassBegin begin;
        begin.FrameIndex = mF2FSync->BeginSynchronize();
        begin.CommandList = mCommandBuffers[begin.FrameIndex];
        begin.SwapchainTexture = mSurface->GetTexture(begin.FrameIndex);
        begin.SwapchainTextureView = mSurface->GetTextureView(begin.FrameIndex);
        begin.Projection = mCamera.Projection();
        begin.View = mCamera.View();

        // Record command list
        RHITextureBarrier endGuiBarrier(begin.SwapchainTexture, RHIResourceAccess::kColorAttachmentWrite, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kColorAttachmentOutput, RHIPipelineStage::kAllCommands, RHIResourceLayout::kPresent);
        RHIRenderBegin renderBegin(mSpecs.WindowWidth, mSpecs.WindowHeight, { RHIRenderAttachment(begin.SwapchainTextureView, false) }, {});

        begin.CommandList->Reset();
        begin.CommandList->Begin();

        // Render
        mRenderer->Render(RenderPath::kBasic, begin);
        
        // ImGui
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->BeginImGui();
        ImGui::ShowDemoWindow();
        begin.CommandList->EndImGui();
        begin.CommandList->EndRendering();
        begin.CommandList->Barrier(endGuiBarrier);
        
        // Synchronize frame
        begin.CommandList->End();
        mF2FSync->EndSynchronize(mCommandBuffers[begin.FrameIndex]);
        mF2FSync->PresentSurface();

        // Take screenshot?
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
            auto tempCmd = mGraphicsQueue->CreateCommandBuffer(true);
            tempCmd->Begin();

            RHITextureBarrier beginTextureBarrier(begin.SwapchainTexture, RHIResourceAccess::kNone, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kBottomOfPipe, RHIPipelineStage::kCopy, RHIResourceLayout::kTransferSrc);
            RHIBufferBarrier beginBufferBarrier(mScreenshotBuffer, RHIResourceAccess::kMemoryRead, RHIResourceAccess::kMemoryWrite, RHIPipelineStage::kAllCommands, RHIPipelineStage::kCopy);
            RHIBarrierGroup beginGroup = {};
            beginGroup.BufferBarriers = { beginBufferBarrier };
            beginGroup.TextureBarriers = { beginTextureBarrier };

            RHITextureBarrier endTextureBarrier(begin.SwapchainTexture, RHIResourceAccess::kMemoryRead, RHIResourceAccess::kNone, RHIPipelineStage::kCopy, RHIPipelineStage::kBottomOfPipe, RHIResourceLayout::kPresent);
            RHIBufferBarrier endBufferBarrier(mScreenshotBuffer, RHIResourceAccess::kMemoryWrite, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kCopy, RHIPipelineStage::kAllCommands);
            RHIBarrierGroup endGroup = {};
            endGroup.BufferBarriers = { endBufferBarrier };
            endGroup.TextureBarriers = { endTextureBarrier };
            
            tempCmd->BarrierGroup(beginGroup);
            tempCmd->CopyTextureToBuffer(mScreenshotBuffer, begin.SwapchainTexture);
            tempCmd->BarrierGroup(endGroup);
            tempCmd->End();
            mGraphicsQueue->SubmitAndFlushCommandBuffer(tempCmd);

            uint8* pixels = (uint8*)mScreenshotBuffer->Map();
            if (mSpecs.Backend == RHIBackend::kVulkan) {
                size_t pixelCount = mSpecs.WindowWidth * mSpecs.WindowHeight;
                for (size_t i = 0; i < pixelCount; ++i) {
                    size_t offset = i * 4;
                    std::swap(pixels[offset + 0], pixels[offset + 2]); // B <-> R
                }
            }

            memcpy(mScreenshotData.Pixels.data(), pixels, mSpecs.WindowWidth * mSpecs.WindowHeight * 4);
            mScreenshotBuffer->Unmap();
            Image::WriteImageData(mScreenshotData, "Data/Screenshot.png");

            delete tempCmd;
        }

        // Update camera
        mCamera.Update(delta, 16, 9);
    }
}
