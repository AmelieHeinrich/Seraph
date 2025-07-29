//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 22:44:11
//

#include "Gfx_Screenshotter.h"
#include "Gfx_CommandListRecycler.h"

#include <KernelCore/KC_FileSystem.h>
#include <chrono>
#include <stb_image_write.h>

namespace Gfx
{
    Screenshotter::Data Screenshotter::sData;

    void Screenshotter::Initialize()
    {
        sData.WantsScreenshot = false;

        sData.ReadbackBuffer = Gfx::Manager::GetDevice()->CreateBuffer(KGPU::BufferDesc(MAX_WIDTH * MAX_HEIGHT * 4, 0, KGPU::BufferUsage::kReadback));
        sData.ReadbackBuffer->SetName("Screenshot Buffer");

        if (!KC::FileSystem::Exists("screenshots/")) {
            KC::FileSystem::CreateDirectory("screenshots/");
        }
    }

    void Screenshotter::Shutdown()
    {
        KC_DELETE(sData.ReadbackBuffer);
    }

    void Screenshotter::EnqueueScreenshot(const char* screenshotID)
    {
        sData.WantsScreenshot = true;
        sData.ScreenshotResourceID = screenshotID;
    }

    bool Screenshotter::WantsScreenshot()
    {
        return sData.WantsScreenshot;
    }

    void Screenshotter::ProcessScreenshot()
    {
        if (!sData.WantsScreenshot)
            return;
        sData.WantsScreenshot = false;

        if (!KC::FileSystem::Exists("screenshots/")) {
            KC::FileSystem::CreateDirectory("screenshots/");
        }

        KGPU::ICommandList* list = Gfx::CommandListRecycler::RequestCommandList();
        Gfx::Resource& screenshotImage = Gfx::ResourceManager::Import(sData.ScreenshotResourceID, list, Gfx::ImportType::kTransferSource);
        list->CopyTextureToBuffer(sData.ReadbackBuffer, screenshotImage.Texture);
        Gfx::CommandListRecycler::FlushCommandLists();

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm* tm = std::localtime(&time_t);
        
        char timestamp[64];
        std::snprintf(timestamp, sizeof(timestamp), 
                      "%04d-%02d-%02d_%02d-%02d-%02d-%03d",
                      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                      tm->tm_hour, tm->tm_min, tm->tm_sec, 
                      static_cast<int>(ms.count()));
        
        std::string screenshotPath = "screenshots/Screenshot_" + std::string(timestamp) + ".png";
        
        // Write
        uint8* pixels = (uint8*)sData.ReadbackBuffer->Map();
        stbi_write_png(screenshotPath.c_str(), screenshotImage.Texture->GetDesc().Width, screenshotImage.Texture->GetDesc().Height, 4, pixels, screenshotImage.Texture->GetDesc().Width * 4);
        sData.ReadbackBuffer->Unmap();

        KD_INFO("Saved screenshot : %s", screenshotPath.c_str());
    }
}
