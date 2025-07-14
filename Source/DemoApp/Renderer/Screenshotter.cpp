//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-14 20:53:46
//

#include "Screenshotter.h"
#include "Passes/Tonemapping.h"

#include <Seraph/Renderer/RendererResourceManager.h>

Screenshotter::Data Screenshotter::sData;

void Screenshotter::Initialize(IRHIDevice* device, IRHICommandQueue* queue, uint width, uint height)
{
    sData.Device = device;
    sData.Queue = queue;
    sData.WantsScreenshot = false;
    sData.Width = width;
    sData.Height = height;

    sData.ReadbackBuffer = device->CreateBuffer(RHIBufferDesc(width * height * 4, 0, RHIBufferUsage::kReadback));
    sData.ReadbackBuffer->SetName("Screenshot Buffer");

    if (!FileSystem::Exists("Screenshots/")) {
        FileSystem::CreateDirectory("Screenshots/");
    }
}

void Screenshotter::Shutdown()
{
    delete sData.ReadbackBuffer;
}

void Screenshotter::EnqueueScreenshot()
{
    sData.WantsScreenshot = true;
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

    if (!FileSystem::Exists("Screenshots/")) {
        FileSystem::CreateDirectory("Screenshots/");
    }

    // Readback
    IRHICommandList* cmdList = sData.Queue->CreateCommandBuffer(true);
    cmdList->Begin();
    RendererResource& screenshotImage = RendererResourceManager::Import(TONEMAPPING_SCREENSHOT_ID, cmdList, RendererImportType::kTransferSource);
    cmdList->CopyTextureToBuffer(sData.ReadbackBuffer, screenshotImage.Texture);
    cmdList->End();
    sData.Queue->SubmitAndFlushCommandBuffer(cmdList);
    delete cmdList;
    
    // Generate timestamp-based filename
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
    
    std::string screenshotPath = "Screenshots/Screenshot_" + std::string(timestamp) + ".png";
    
    // Write
    uint8* pixels = (uint8*)sData.ReadbackBuffer->Map();
    Image::WriteImageData(pixels, sData.Width, sData.Height, screenshotPath.c_str());
    sData.ReadbackBuffer->Unmap();
}
