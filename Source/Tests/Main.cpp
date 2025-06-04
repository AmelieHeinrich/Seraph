//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-04 18:00:44
//

#include "Test.h"
#include "FLIP.h"

float SRGBToLinear(uint8 srgb)
{
    float c = srgb / 255.0f;
    return (c <= 0.04045f) ? (c / 12.92f) : powf((c + 0.055f) / 1.055f, 2.4f);
}

void ConvertFLIP(const uint8* rgba8, float* linearRGB, int width, int height)
{
    for (int i = 0; i < width * height; ++i) {
        linearRGB[i * 3 + 0] = SRGBToLinear(rgba8[i * 4 + 0]);
        linearRGB[i * 3 + 1] = SRGBToLinear(rgba8[i * 4 + 1]);
        linearRGB[i * 3 + 2] = SRGBToLinear(rgba8[i * 4 + 2]);
    }
}

std::string StringifyBackend(RHIBackend backend)
{
    switch (backend)
    {
        case RHIBackend::kVulkan: return "Vulkan";
        case RHIBackend::kD3D12: return "D3D12";
    }
    return "WTF";
}

int main(void)
{
    Context::Initialize();

    auto& tests = GetTests();
    int totalPassed = 0;

    Array<String> passed;
    Array<String> failed;

    for (auto* test : tests) {
        TestResult vulkanData = test->Run(RHIBackend::kVulkan);
        std::string vulkanPath = "Data/Tests/" + std::string(test->Name()) + std::string("Vulkan") + ".png";
        Image::WriteImageData(vulkanData.Data, vulkanPath);
        vulkanData.Data.Pixels.clear();
        float* linearVulkan = new float[TEST_WIDTH * TEST_HEIGHT * 3];
        ConvertFLIP(vulkanData.Data.Pixels.data(), linearVulkan, TEST_WIDTH, TEST_HEIGHT);

        TestResult d3dData = test->Run(RHIBackend::kD3D12);
        std::string d3dPath = "Data/Tests/" + std::string(test->Name()) + std::string("D3D12") + ".png";
        Image::WriteImageData(d3dData.Data, d3dPath);
        d3dData.Data.Pixels.clear();
        float* linearD3D = new float[TEST_WIDTH * TEST_HEIGHT * 3];
        ConvertFLIP(d3dData.Data.Pixels.data(), linearD3D, TEST_WIDTH, TEST_HEIGHT);

        std::string magmaPath = "Data/Tests/" + std::string(test->Name()) + "Magma" + ".png";

        FLIP::Parameters parameters;
        float meanError;
        float* outMagma;
        FLIP::evaluate(linearVulkan, linearD3D, TEST_WIDTH, TEST_HEIGHT, false, parameters, true, true, meanError, &outMagma);
        Image::WriteImageRGB(outMagma, TEST_WIDTH, TEST_HEIGHT, magmaPath);

        if (meanError > 0.02f) {
            failed.push_back(String(test->Name()));
        } else {
            passed.push_back(String(test->Name()));
            totalPassed++;
        }
        delete outMagma;
        delete[] linearVulkan;
        delete[] linearD3D;
    }

    for (auto& pass : passed) {
        SERAPH_INFO("%s PASSED", pass.c_str());
    }
    for (auto& fail : failed) {
        SERAPH_ERROR("%s FAILED", fail.c_str());
    }
    SERAPH_INFO("Total Passed: %d/%d", totalPassed, tests.size());
    Context::Shutdown();
}
