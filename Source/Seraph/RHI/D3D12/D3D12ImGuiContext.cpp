//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 23:02:52
//

#include "D3D12ImGuiContext.h"
#include "D3D12Device.h"
#include "D3D12CommandQueue.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGui/imgui_impl_dx12.h>

D3D12ImGuiContext::D3D12ImGuiContext(D3D12Device* device, D3D12CommandQueue* queue, Window* window)
    : mParentDevice(device)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    ImGui::StyleColorsDark();

    mFontAlloc = device->GetBindlessManager()->FindFreeSpace();

    ImGui_ImplDX12_InitInfo initInfo = {};
    initInfo.Device = device->GetDevice();
    initInfo.CommandQueue = queue->GetQueue();
    initInfo.NumFramesInFlight = FRAMES_IN_FLIGHT;
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN;
    initInfo.SrvDescriptorHeap = device->GetBindlessManager()->GetResourceHeap();
    initInfo.UserData = device->GetBindlessManager();
    initInfo.LegacySingleSrvCpuDescriptor = mFontAlloc.CPU;
    initInfo.LegacySingleSrvGpuDescriptor = mFontAlloc.GPU;
    
    ImGui_ImplSDL3_InitForVulkan(window->GetWindow());
    ImGui_ImplDX12_Init(&initInfo);
    ImGui_ImplDX12_CreateDeviceObjects();
}

D3D12ImGuiContext::~D3D12ImGuiContext()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    mParentDevice->GetBindlessManager()->FreeCBVSRVUAV(mFontAlloc);   
}
