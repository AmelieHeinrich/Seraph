//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 22:56:00
//

#include "VulkanImGuiContext.h"
#include "VulkanDevice.h"
#include "VulkanCommandQueue.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGui/imgui_impl_vulkan.h>

VulkanImGuiContext::VulkanImGuiContext(VulkanDevice* device, VulkanCommandQueue* queue, Window* window)
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    ImGui::StyleColorsDark();

    VkFormat formats[] = { VK_FORMAT_B8G8R8A8_UNORM };

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = device->Instance();
    initInfo.PhysicalDevice = device->GPU();
    initInfo.Device = device->Device();
    initInfo.QueueFamily = device->GraphicsQueueFamilyIndex();
    initInfo.Queue = queue->GetQueue();
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = formats;
    initInfo.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    initInfo.Subpass = 0;
    initInfo.MinImageCount = FRAMES_IN_FLIGHT;
    initInfo.ImageCount = FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.DescriptorPoolSize = 100;
    
    io.FontDefault = io.Fonts->AddFontFromFileTTF("Data/Fonts/UIFont.ttf", 16);

    ImGui_ImplSDL3_InitForVulkan(window->GetWindow());
    ImGui_ImplVulkan_Init(&initInfo);
    ImGui_ImplVulkan_CreateFontsTexture();
}

VulkanImGuiContext::~VulkanImGuiContext()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
