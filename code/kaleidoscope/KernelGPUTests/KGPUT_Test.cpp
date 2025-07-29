//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-29 18:50:15
//

#include "KGPUT_Test.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_Mipmapper.h>
#include <Graphics/Gfx_ResourceManager.h>
#include <Graphics/Gfx_TempResourceStorage.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <Graphics/Gfx_CommandListRecycler.h>

#include <KDShader/KDS_Manager.h>

namespace KGPUT
{
    TestData ITest::CreateData()
    {
        TestData data;

        data.Device = KGPU::IDevice::Create(false);
        data.CommandQueue = data.Device->CreateCommandQueue(KGPU::CommandQueueType::kGraphics);

        KGPU::TextureDesc renderDesc = {};
        renderDesc.Format = KGPU::TextureFormat::kR8G8B8A8_UNORM;
        renderDesc.Width = TEST_WIDTH;
        renderDesc.Height = TEST_HEIGHT;
        renderDesc.Usage = KGPU::TextureUsage::kRenderTarget | KGPU::TextureUsage::kShaderResource | KGPU::TextureUsage::kStorage;

        data.RenderTexture = data.Device->CreateTexture(renderDesc);
        data.ScreenshotBuffer = data.Device->CreateBuffer(KGPU::BufferDesc(TEST_WIDTH * TEST_HEIGHT * 4, sizeof(uint), KGPU::BufferUsage::kReadback));

        KDS::Manager::Initialize();
        Gfx::Manager::Initialize(data.Device, data.CommandQueue);
        Gfx::ShaderManager::Initialize();
        Gfx::Mipmapper::Initialize();
        Gfx::ResourceManager::Initialize();
        
        return data;
    }

    void ITest::DeleteData(TestData& data)
    {
        Gfx::CommandListRecycler::FlushCommandLists();
        Gfx::CommandListRecycler::Clean();
        Gfx::ViewRecycler::Clean();
        Gfx::TempResourceStorage::Clean();
        Gfx::ShaderManager::Shutdown();
        Gfx::ResourceManager::Shutdown();
        KDS::Manager::Shutdown();

        KC_DELETE(data.ScreenshotBuffer);
        KC_DELETE(data.RenderTexture);
        KC_DELETE(data.CommandQueue);
        KC_DELETE(data.Device);
    }

    KC::Array<ITest*>& GetTests()
    {
        static KC::Array<ITest*> tests;
        return tests;
    }

    void RegisterTest(ITest* test)
    {
        GetTests().push_back(test);
    }
}
