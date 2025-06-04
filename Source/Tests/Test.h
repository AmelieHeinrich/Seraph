//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 18:01:58
//

#pragma once

#include <Seraph/Seraph.h>

constexpr uint TEST_WIDTH = 1280;
constexpr uint TEST_HEIGHT = 720;

struct TestStarters
{
    IRHIDevice* Device;
    IRHICommandQueue* Queue;

    IRHITexture* RenderTexture;
    IRHIBuffer* ScreenshotBuffer;
    ImageData ScreenshotData;
};

struct TestResult
{
    ImageData Data;
    bool Result;
};

class ITest
{
public:
    static TestStarters CreateStarters(RHIBackend backend);
    static void DeleteStarts(TestStarters starters);

    virtual const char* Name() const = 0;
    virtual TestResult Run(RHIBackend backend) = 0;
};

Array<ITest*>& GetTests();
void RegisterTest(ITest* test);

#define DEFINE_RHI_TEST(testName) \
    struct testName : public ITest { \
        const char* Name() const override { return #testName; } \
        TestResult Run(RHIBackend backend) override; \
    }; \
    static testName testName##_instance; \
    static bool testName##_registered = ([](){ RegisterTest(&testName##_instance); return true; })(); \
    TestResult testName::Run(RHIBackend backend)
