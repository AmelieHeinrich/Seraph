//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-26 19:21:37
//

#include <Seraph/Core/Context.h>
#include <Seraph/Core/FileSystem.h>

#include "Application.h"

RHIBackend TranslateBackend(const String& data)
{
    if (data == "Vulkan") return RHIBackend::kVulkan;
    if (data == "D3D12") return RHIBackend::kD3D12;
    return RHIBackend::kD3D12;
}

int main(void)
{
    Context::Initialize();
    FileSystem::Initialize();

    {
        nlohmann::json settings = FileSystem::ReadJSON("Data/Settings.json");
        
        ApplicationSpecs specs = { TranslateBackend(settings["backend"]), settings["width"], settings["height"] };
        Application app(specs);
        app.Run();
    }
    
    FileSystem::Shutdown();
    Context::Shutdown();
    return 0;
}
