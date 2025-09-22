//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:04:39
//

#include "SP_Application.h"

#include <ToolImGui/ToolImGui_Manager.h>
#include <ToolDevConsole/TDC_Console.h>
#include <KernelInput/KI_InputSystem.h>
#include <KDAsset/KDA_TextureLoader.h>
#include <KDAsset/KDA_MeshLoader.h>
#include <KDShader/KDS_Manager.h>
#include <Effects/FX_DebugRenderer.h>
#include <Graphics/Gfx_TempResourceStorage.h>
#include <Graphics/Gfx_CommandListRecycler.h>
#include <Graphics/Gfx_MeshPrimitive.h>
#include <Graphics/Gfx_Material.h>
#include <Graphics/Gfx_Manager.h>
#include <Graphics/Gfx_ResourceManager.h>
#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_Uploader.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_Skybox.h>
#include <Graphics/Gfx_Mipmapper.h>
#include <Graphics/Gfx_Uploader.h>
#include <Graphics/Gfx_Screenshotter.h>

#include <imgui.h>
#include <fontawesome.h>

#include "Renderer/Hybrid/SP_Tonemap.h"

namespace SP
{
    Application* Application::sInstance;

    Application::Application()
    {
        sInstance = this;
        TDC::Console::Initialize();
        mWindow = KOS::IWindow::Create(mWidth, mHeight, "Seraph | Kaleidoscope 0.0.1");

        CODE_BLOCK("Create RHI objects") {
            mDevice = KGPU::IDevice::Create(false, KGPU::Backend::kAuto);
            mCommandQueue = mDevice->CreateCommandQueue(KGPU::CommandQueueType::kGraphics);
            mSurface = mDevice->CreateSurface(mWindow, mCommandQueue);
            for (int i = 0; i < KGPU::FRAMES_IN_FLIGHT; i++) {
                mLists[i] = mCommandQueue->CreateCommandList(false);
            }
            mFrameSync = mDevice->CreateSync(mSurface, mCommandQueue);
        
            switch (mDevice->GetBackend()) {
                case KGPU::Backend::kD3D12: mStringBackend = "D3D12"; break;
                case KGPU::Backend::kVulkan: mStringBackend = "Vulkan"; break;
                case KGPU::Backend::kMetal: mStringBackend = "Metal"; break;
                default: mStringBackend = "Sign an NDA first..."; break;
            }
        }

        CODE_BLOCK("Initialize systems") {
            KI::InputSystem::Initialize();
            KDS::Manager::Initialize();
            Gfx::Manager::Initialize(mDevice, mCommandQueue);
            Gfx::ResourceManager::Initialize();
            Gfx::ShaderManager::Initialize();
            Gfx::SkyboxCooker::Initialize();
            Gfx::Mipmapper::Initialize();
            Gfx::Screenshotter::Initialize();
            ToolImGui::Manager::Initialize(mWindow, mDevice);
            ToolImGui::Manager::BuildRenderer();

            mRenderer = KC_NEW(WorldRenderer);
            mRenderer->Prepare();
        }

        CODE_BLOCK("Create world") {
            mWorld = KC_NEW(RenderWorld);
            mWorld->AddMesh("data/sp/models/DamagedHelmet/DamagedHelmet.gltf");
            mWorld->GetLightList()->Sun.Direction = glm::vec3(0.0f, -1.0f, 0.0f);
            mWorld->GetLightList()->Sun.Intensity = 10.0f;
            mWorld->GetLightList()->Sun.Color = glm::vec3(1.0f);

            Gfx::SkyboxCooker::GenerateSkybox(mSky, "data/sp/sky/snow.hdr");
            TDC::Console::AddFunction("Seraph.LoadSkybox", [&](const KC::String& args){
                if (KC::FileSystem::Exists(args)) {
                    mSkyboxReloadPath = args;
                    mPendingSkyboxReload = true;
                } else {
                    KD_ERROR("Cannot load skybox %s - it doesn't exist.", args.c_str());
                }
            });
        }

        CODE_BLOCK("Finish start and go!") {
            Gfx::CommandListRecycler::FlushCommandLists();
            Gfx::TempResourceStorage::Clean();
        }

        TDC::Console::AddFunction("Seraph.ReloadShaders", [&](const KC::String&){
            mPendingShaderReload = true;
        });
    
        KD_INFO("Seraph ready!");
    }

    Application::~Application()
    {
        mSky.Clean();
        KC_DELETE(mRenderer);
        KC_DELETE(mWorld);

        ToolImGui::Manager::Shutdown();
        Gfx::Screenshotter::Shutdown();
        Gfx::TempResourceStorage::Clean();
        Gfx::ViewRecycler::Clean();
        Gfx::ShaderManager::Shutdown();
        Gfx::ResourceManager::Shutdown();
        Gfx::CommandListRecycler::Clean();

        KC_DELETE(mFrameSync);
        for (int i = 0; i < KGPU::FRAMES_IN_FLIGHT; i++) {
            KC_DELETE(mLists[i]);
        }
        KC_DELETE(mSurface);
        KC_DELETE(mCommandQueue);
        KC_DELETE(mDevice);

        KOS::Delete(mWindow);
    
        KDS::Manager::Shutdown();
        KI::InputSystem::Shutdown();
    }

    void Application::Run()
    {
        while (mWindow->IsOpen()) {
            double now = KC::GlobalTimer.ToSeconds();
            double dt = now - mLast;
            mLast = now;

            CODE_BLOCK("Render") {
                mWorld->Update();

                mWindow->GetSize(mBegin.Width, mBegin.Height);
                mBegin.FrameIndex = mFrameSync->BeginSynchronize();
                mBegin.CmdList = mLists[mBegin.FrameIndex];
                mBegin.SwapTexture = mSurface->GetTexture(mBegin.FrameIndex);
                mBegin.SwapView = mSurface->GetTextureView(mBegin.FrameIndex);
                mBegin.World = mWorld;
                mBegin.Sky = &mSky;
                mBegin.CamData.Proj = mCamera.Projection();
                mBegin.CamData.View = mCamera.View();
                mBegin.CamData.ViewProj = mBegin.CamData.Proj * mBegin.CamData.View;
                mBegin.CamData.InvProj = glm::inverse(mBegin.CamData.Proj);
                mBegin.CamData.InvView = glm::inverse(mBegin.CamData.View);
                mBegin.CamData.InvViewProj = glm::inverse(mBegin.CamData.Proj * mBegin.CamData.View);
                mBegin.CamData.Position = float4(mCamera.Position(), 1.0f);
                mBegin.FrameCount++;

                mBegin.CmdList->Reset();
                mBegin.CmdList->Begin();
                mRenderer->Render(mBegin);
                mBegin.CmdList->BeginRendering(KGPU::RenderBegin(mBegin.Width, mBegin.Height, { KGPU::RenderAttachment(mBegin.SwapView, false) }, {}));
                ToolImGui::Manager::Begin();
                UI();
                TDC::Console::Draw(dt, mWidth, mHeight);
                ToolImGui::Manager::Render(mBegin.CmdList, mBegin.FrameIndex);
                mBegin.CmdList->EndRendering();
                mBegin.CmdList->Barrier(KGPU::TextureBarrier(
                    mBegin.SwapTexture,
                    KGPU::ResourceAccess::kColorAttachmentWrite,
                    KGPU::ResourceAccess::kMemoryRead,
                    KGPU::PipelineStage::kColorAttachmentOutput,
                    KGPU::PipelineStage::kAllCommands,
                    KGPU::ResourceLayout::kPresent
                ));
                mBegin.CmdList->End();

                mFrameSync->EndSynchronize(mBegin.CmdList);
                mFrameSync->PresentSurface();

                if (mPendingShaderReload) {
                    Gfx::ShaderManager::ReloadPipelines(true);
                    mPendingShaderReload = false;
                }
                if (mPendingSkyboxReload) {
                    mSky.Clean();
                    mPendingSkyboxReload = false;
                
                    Gfx::SkyboxCooker::GenerateSkybox(mSky, mSkyboxReloadPath);
                    Gfx::CommandListRecycler::FlushCommandLists();
                    Gfx::TempResourceStorage::Clean();
                }
            }

            CODE_BLOCK("Reset") {
                KI::InputSystem::Reset();
            }

            CODE_BLOCK("Update") {
                void* event;
                while (mWindow->PollEvents(&event)) {
                    ToolImGui::Manager::Update(event);
                }

                ImGuiIO& io = ImGui::GetIO();
                if (!io.WantCaptureKeyboard && !io.WantCaptureMouse) {
                    mCamera.Update(dt, mWidth, mHeight);
                }

                Gfx::Screenshotter::ProcessScreenshot();
                Gfx::ShaderManager::ReloadPipelines();

                mBegin.CamData.PrevProj = mBegin.CamData.Proj;
                mBegin.CamData.PrevView = mBegin.CamData.View;
                mBegin.CamData.PrevViewProj = mBegin.CamData.ViewProj;
                mBegin.CamData.PrevInvProj = mBegin.CamData.InvProj;
                mBegin.CamData.PrevInvView = mBegin.CamData.InvView;
                mBegin.CamData.PrevInvViewProj = mBegin.CamData.InvViewProj;
            }
        }
        mCommandQueue->Wait();
    }

    void Application::UI()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style.FontScaleMain = mFontScale;

        if (KI::InputSystem::IsKeyPressed(KI::Keycode::kF1)) {
            mUIOpened = !mUIOpened;
        }
        if (KI::InputSystem::IsKeyPressed(KI::Keycode::kF3)) {
            mOverlayOpened = !mOverlayOpened;
        }

        if (mUIOpened) {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu(ICON_FA_WINDOWS " Window")) {
                    ImGui::SliderFloat("Font Scale", &mFontScale, 0.5f, 2.0f);

                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu(ICON_FA_VIDEO_CAMERA " Renderer")) {
                    if (ImGui::MenuItem(ICON_FA_WRENCH " Settings")) {
                        mRendererSettingsOpened = !mRendererSettingsOpened;
                    }
                    if (ImGui::MenuItem(ICON_FA_FIRE " Hot Reload Shaders")) {
                        mPendingShaderReload = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // Overlay
            if (mOverlayOpened) {
                ImGuiIO& io = ImGui::GetIO();
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |    ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking;
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
                ImGui::Text(ICON_FA_LAPTOP " Backend : %s", mStringBackend.c_str());
                ImGui::Text(ICON_FA_WRENCH " Has RT: %d - Has MS: %d", Gfx::Manager::GetDevice()->SupportsRaytracing(), Gfx::Manager::GetDevice()->SupportsMeshShaders());
                ImGui::Separator();
                ImGui::Text(ICON_FA_CODEPEN " Debug Menu: F1");
                ImGui::Text(ICON_FA_CAMERA " Screenshot: F2");
                ImGui::Text(ICON_FA_WINDOW_CLOSE " Hide Overlay: F3");
                ImGui::End();
            }

            // Renderer settings
            if (mRendererSettingsOpened) {
                mRenderer->UI(mBegin);
            }
        }
    }
}
