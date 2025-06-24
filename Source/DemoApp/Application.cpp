//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 19:24:03
//

#include "Application.h"
#include "Renderer/Passes/Tonemapping.h"
#include "Renderer/Passes/Debug.h"

#include <chrono>
#include <glm/gtc/type_ptr.hpp>

Application::Application(const ApplicationSpecs& specs)
    : mSpecs(specs), mPath(RenderPath::kPathtracer)
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

    Random rng(12345);
    for (int i = 0; i < 2048; i++) {
        mScene->GetLights().AddPointLight(
            rng.Vec3(float3(-10.0f, 0.0f, -7.0f), float3(10.0f, 10.0f, 7.0f)),
            rng.Float(0.5f, 2.0f),
            rng.Vec3(float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f)),
            rng.Float(1.0f, 5.0f)
        );
    }

    Uploader::Flush();

    SetUITheme();
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
        begin.CamData.Position = float4(mCamera.Position(), 1.0f);

        // Record command list
        RHITextureBarrier endGuiBarrier(begin.SwapchainTexture, RHIResourceAccess::kColorAttachmentWrite, RHIResourceAccess::kMemoryRead, RHIPipelineStage::kColorAttachmentOutput, RHIPipelineStage::kAllCommands, RHIResourceLayout::kPresent);
        RHIRenderBegin renderBegin(mSpecs.WindowWidth, mSpecs.WindowHeight, { RHIRenderAttachment(begin.SwapchainTextureView, false) }, {});

        begin.CommandList->Reset();
        begin.CommandList->Begin();

        // Render
        mRenderer->Render(mPath, begin);
        
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
        if (!mUIOpened) {
            mCamera.Update(delta, 16, 9);
        }
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
                if (ImGui::MenuItem("Settings")) {
                    mRendererSettingsOpened = !mRendererSettingsOpened;
                }
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
            ImGui::Text("Screenshot: (DISABLED)");
            ImGui::Text("Hide Overlay: F3");
            ImGui::Separator();
            if (ImGui::RadioButton("Rasterized", mPath == RenderPath::kBasic)) mPath = RenderPath::kBasic;
            if (ImGui::RadioButton("Pathtraced", mPath == RenderPath::kPathtracer)) mPath = RenderPath::kPathtracer;
            ImGui::End();
        }

        // Renderer settings
        if (mRendererSettingsOpened) {
            mRenderer->UI(mPath, begin);
        }
    }
}

void Application::SetUITheme()
{
    ImGuiStyle& style = ImGui::GetStyle();
	
	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6000000238418579f;
	style.WindowPadding = ImVec2(5.5f, 8.300000190734863f);
	style.WindowRounding = 4.5f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 3.200000047683716f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 2.700000047683716f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 2.400000095367432f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 3.200000047683716f;
	style.TabRounding = 3.5f;
	style.TabBorderSize = 1.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
	
	style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.4980392158031464f, 0.4980392158031464f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 0.9399999976158142f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.0784313753247261f, 0.0784313753247261f, 0.9399999976158142f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1372549086809158f, 0.1725490242242813f, 0.2274509817361832f, 0.5400000214576721f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2117647081613541f, 0.2549019753932953f, 0.3019607961177826f, 0.4000000059604645f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.04313725605607033f, 0.0470588244497776f, 0.0470588244497776f, 0.6700000166893005f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0784313753247261f, 0.08235294371843338f, 0.09019608050584793f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1372549086809158f, 0.1372549086809158f, 0.1372549086809158f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.01960784383118153f, 0.01960784383118153f, 0.01960784383118153f, 0.5299999713897705f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3098039329051971f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.407843142747879f, 0.407843142747879f, 0.407843142747879f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5098039507865906f, 0.5098039507865906f, 0.5098039507865906f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7176470756530762f, 0.7843137383460999f, 0.843137264251709f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47843137383461f, 0.5254902243614197f, 0.572549045085907f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2901960909366608f, 0.3176470696926117f, 0.3529411852359772f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.1490196138620377f, 0.1607843190431595f, 0.1764705926179886f, 0.4000000059604645f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1372549086809158f, 0.1450980454683304f, 0.1568627506494522f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.09019608050584793f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.196078434586525f, 0.2156862765550613f, 0.239215686917305f, 0.3100000023841858f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1647058874368668f, 0.1764705926179886f, 0.1921568661928177f, 0.800000011920929f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.07450980693101883f, 0.08235294371843338f, 0.09019608050584793f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.4274509847164154f, 0.4274509847164154f, 0.4980392158031464f, 0.5f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.239215686917305f, 0.3254902064800262f, 0.4235294163227081f, 0.7799999713897705f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.2745098173618317f, 0.3803921639919281f, 0.4980392158031464f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2901960909366608f, 0.3294117748737335f, 0.3764705955982208f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.239215686917305f, 0.2980392277240753f, 0.3686274588108063f, 0.6700000166893005f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.1647058874368668f, 0.1764705926179886f, 0.1882352977991104f, 0.949999988079071f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.1176470592617989f, 0.125490203499794f, 0.1333333402872086f, 0.8619999885559082f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.3294117748737335f, 0.407843142747879f, 0.501960813999176f, 0.800000011920929f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2431372553110123f, 0.2470588237047195f, 0.2549019753932953f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.3499999940395355f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}
