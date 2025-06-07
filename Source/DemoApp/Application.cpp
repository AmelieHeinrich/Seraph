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

    mRenderer = new Renderer(mDevice, mSpecs.WindowWidth, mSpecs.WindowHeight);
    
    Uploader::Flush();
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mRenderer;
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

        // Update camera
        mCamera.Update(delta, 16, 9);
    }
}
