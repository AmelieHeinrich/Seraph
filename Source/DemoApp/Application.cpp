//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"

#include <imgui/imgui.h>
#include <chrono>

Application::Application(const ApplicationSpecs& specs)
    : mSpecs(specs)
{
    Compressor compressor;
    compressor.RecurseFolder("Data/");

    ShaderCompiler::Initialize(specs.Backend);

    mDevice = IRHIDevice::CreateDevice(specs.Backend, true);
    AssetManager::Initialize(mDevice);
    mGraphicsQueue = mDevice->CreateCommandQueue(RHICommandQueueType::kGraphics);
    Uploader::Initialize(mDevice, mGraphicsQueue);

    mWindow = SharedPtr<Window>(new Window(specs.WindowWidth, specs.WindowHeight, "Seraph"));
    mSurface = mDevice->CreateSurface(mWindow.get(), mGraphicsQueue);
    mF2FSync = mDevice->CreateF2FSync(mSurface, mGraphicsQueue);
    mImGuiContext = mDevice->CreateImGuiContext(mGraphicsQueue, mWindow.get());
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        mCommandBuffers[i] = mGraphicsQueue->CreateCommandBuffer(false);
    }

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

    AssetManager::Shutdown();
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
