//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 18:13:42
//

#include "Test.h"

TestStarters ITest::CreateStarters(RHIBackend backend)
{
    TestStarters result = {};
    result.Device = IRHIDevice::CreateDevice(backend, true);
    result.Queue = result.Device->CreateCommandQueue(RHICommandQueueType::kGraphics);
    
    RHITextureDesc renderDesc = {};
    renderDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
    renderDesc.Width = TEST_WIDTH;
    renderDesc.Height = TEST_HEIGHT;
    renderDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource | RHITextureUsage::kStorage;
    result.RenderTexture = result.Device->CreateTexture(renderDesc);

    result.ScreenshotBuffer = result.Device->CreateBuffer(RHIBufferDesc(renderDesc.Width * renderDesc.Height * 4, 0, RHIBufferUsage::kReadback));
    result.ScreenshotData.Width = renderDesc.Width;
    result.ScreenshotData.Height = renderDesc.Height;
    result.ScreenshotData.Pixels.resize(renderDesc.Width * renderDesc.Height * 4);

    ShaderCompiler::Initialize(backend);
    Uploader::Initialize(result.Device, result.Queue);

    return result;
}

void ITest::DeleteStarts(TestStarters starters)
{
    Uploader::Shutdown();
    ShaderCompiler::Shutdown();

    delete starters.RenderTexture;
    delete starters.ScreenshotBuffer;
    delete starters.Queue;
    delete starters.Device;
}

Array<ITest*>& GetTests()
{
    static Array<ITest*> tests;
    return tests;
}

void RegisterTest(ITest* test)
{
    GetTests().push_back(test);
}
