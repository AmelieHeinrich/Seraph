//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"
#include "Renderer/Passes/Tonemapping.h"
#include "Renderer/Passes/Debug.h"

#include <chrono>

Application::Application(const ApplicationSpecs& specs)
    : mSpecs(specs)
{
    mStringBackend = specs.Backend == RHIBackend::kVulkan ? "Vulkan" : "D3D12";

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

    mScene = new Scene(mDevice);
    mScene->AddEntity("Data/Models/Sponza/Sponza.gltf");

    Random rng;
    for (int i = 0; i < 256; i++) {
        mScene->GetLights().AddPointLight(
            rng.Vec3(glm::vec3(-10.0f, 0.0f, -5.0f), glm::vec3(10.0f, 7.0f, 5.0f)),
            rng.Float(0.5f, 2.0f),
            rng.Vec3(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
            rng.Float(1.0f, 10.0f)
        );
    }

    Uploader::Flush();
}

Application::~Application()
{
    Uploader::Shutdown();

    delete mScene;
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
        begin.RenderScene = mScene;
        begin.CamData.Proj = mCamera.Projection();
        begin.CamData.View = mCamera.View();
        begin.CamData.ViewProj = begin.CamData.Proj * begin.CamData.View;
        begin.CamData.InvProj = glm::inverse(begin.CamData.Proj);
        begin.CamData.InvView = glm::inverse(begin.CamData.View);
        begin.CamData.InvViewProj = glm::inverse(begin.CamData.Proj * begin.CamData.View);
        begin.CamData.Position = glm::vec4(mCamera.Position(), 1.0f);

        // Record command list
        RHITextureBarrier endGuiBarrier(begin.SwapchainTexture, RHIResourceAccess::kColorAttachmentWrite, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kColorAttachmentOutput, RHIPipelineStage::kAllCommands, RHIResourceLayout::kPresent);
        RHIRenderBegin renderBegin(mSpecs.WindowWidth, mSpecs.WindowHeight, { RHIRenderAttachment(begin.SwapchainTextureView, false) }, {});

        begin.CommandList->Reset();
        begin.CommandList->Begin();

        // Draw lights
        // for (auto& light : mScene->GetLights().PointLights) {
        //     Debug::DrawRings(light.Position, light.Radius, light.Color);
        // }

        // Render
        mRenderer->Render(RenderPath::kBasic, begin);
        
        // ImGui
        begin.CommandList->PushMarker("ImGui");
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->BeginImGui();
        UI(begin);
        begin.CommandList->EndImGui();
        begin.CommandList->EndRendering();
        begin.CommandList->Barrier(endGuiBarrier);
        begin.CommandList->PopMarker();
        
        // Synchronize frame
        begin.CommandList->End();
        mF2FSync->EndSynchronize(mCommandBuffers[begin.FrameIndex]);
        mF2FSync->PresentSurface();

        // Update
        mCamera.Update(delta, 16, 9);
        mScene->Update(begin.FrameIndex);
    }
}

void Application::UI(RenderPassBegin& begin)
{
    if (ImGui::IsKeyPressed(ImGuiKey_F1, false)) {
        mUIOpened = !mUIOpened;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_F3, false)) {
        mOverlayOpened = !mOverlayOpened;
    }

    if (mUIOpened) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("World")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Renderer")) {
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Overlay
        if (mOverlayOpened) {
            ImGuiIO& io = ImGui::GetIO();
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking;
            const float PAD = 10.0f;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 work_pos = viewport->WorkPos;
            ImVec2 work_size = viewport->WorkSize;
            ImVec2 window_pos, window_pos_pivot;
            window_pos.x = (work_pos.x + PAD);
            window_pos.y = (work_pos.y + PAD);
            window_pos_pivot.x = 0.0f;
            window_pos_pivot.y = 0.0f;
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
            window_flags |= ImGuiWindowFlags_NoMove;
            
            static bool p_open = true;
            ImGui::SetNextWindowBgAlpha(0.70f);
            ImGui::Begin("Example: Simple overlay", &p_open, window_flags);
            ImGui::Text("Seraph - A modern graphics renderer by Amelie Heinrich");
            ImGui::Text("Backend : %s", mStringBackend.c_str());
            ImGui::Separator();
            ImGui::Text("Debug Menu: F1");
            ImGui::Text("Screenshot: F2");
            ImGui::Text("Hide Overlay: F3");
            ImGui::End();
        }
    }
}
