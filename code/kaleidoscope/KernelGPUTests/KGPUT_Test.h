//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-29 18:47:32
//

#pragma once

#include <Graphics/Gfx_Manager.h>

namespace KGPUT
{
    constexpr uint TEST_WIDTH = 1280;
    constexpr uint TEST_HEIGHT = 720;

    struct TestData
    {
        KGPU::IDevice* Device;
        KGPU::ICommandQueue* CommandQueue;

        KGPU::ITexture* RenderTexture;
        KGPU::IBuffer* ScreenshotBuffer;
        KC::Array<uint8> ScreenshotData;
    };

    class ITest
    {
    public:
        virtual ~ITest() = default;

        static TestData CreateData();
        static void DeleteData(TestData& data);

        virtual const char* Name() const = 0;
        virtual KC::Array<uint8> Run() = 0;
    };

    KC::Array<ITest*>& GetTests();
    void RegisterTest(ITest* test);
}

#define DEFINE_RHI_TEST(testName) \
    struct testName : public KGPUT::ITest { \
        const char* Name() const override { return #testName; } \
        KC::Array<uint8> Run() override; \
    }; \
    static bool testName##_registered = ([](){ RegisterTest(new testName); return true; })(); \
    KC::Array<uint8> testName::Run()
