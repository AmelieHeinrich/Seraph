//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 23:02:52
//

#include "MetalImGuiContext.h"
#include "MetalDevice.h"
#include "MetalCommandQueue.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl3.h>

MetalImGuiContext::MetalImGuiContext(MetalDevice* device, MetalCommandQueue* queue, Window* window)
    : mParentDevice(device)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    ImGui::StyleColorsDark();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("Data/Fonts/UIFont.ttf", 16);
    
    ImGui_ImplSDL3_InitForVulkan(window->GetWindow());
}

MetalImGuiContext::~MetalImGuiContext()
{
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
