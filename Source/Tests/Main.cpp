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
        case RHIBackend::kDummy: return "Dummy";
        case RHIBackend::kMetal: return "Metal";
    }
    return "WTF";
}

int main(void)
{
    Context::Initialize();

    auto& tests = GetTests();
    nlohmann::json json;

    auto StripDataPrefix = [](const std::string& path) -> std::string {
        const std::string prefix = "Data/";
        if (path.rfind(prefix, 0) == 0) { // starts with "Data/"
            return path.substr(prefix.length());
        }
        return path;
    };

    int testCount = tests.size();
    int testPassed = 0;
    for (auto* test : tests) {
        std::string goldenPath = "Data/Tests/Golden/" + std::string(test->Name()) + "Golden.png";
        std::string magmaVulkanPath = "Data/Tests/" + std::string(test->Name()) + "MagmaVulkan.png";
        std::string magmaD3DPath = "Data/Tests/" + std::string(test->Name()) + "MagmaD3D12.png";
        std::string magmaMetalPath = "Data/Tests/" + std::string(test->Name()) + "MagmaMetal.png";
        std::string vulkanPath = "Data/Tests/" + std::string(test->Name()) + "Vulkan.png";
        std::string d3dPath = "Data/Tests/" + std::string(test->Name()) + "D3D12.png";
        std::string metalPath = "Data/Tests/" + std::string(test->Name()) + "Metal.png";

        ImageData golden = Image::LoadImageData(goldenPath);
        std::vector<float> linearGolden(TEST_WIDTH * TEST_HEIGHT * 3);
        ConvertFLIP(golden.Pixels.data(), linearGolden.data(), TEST_WIDTH, TEST_HEIGHT);

        float finalMean = 0.0f;
        int backendCount = 0;

    #ifdef SERAPH_WINDOWS
        TestResult vulkanData = test->Run(RHIBackend::kVulkan);
        Image::WriteImageData(vulkanData.Data, vulkanPath);
        std::vector<float> linearVulkan(TEST_WIDTH * TEST_HEIGHT * 3);
        ConvertFLIP(vulkanData.Data.Pixels.data(), linearVulkan.data(), TEST_WIDTH, TEST_HEIGHT);

        {
            FLIP::Parameters parameters;
            float meanError;
            float* outMagmaRaw = nullptr;
            FLIP::evaluate(linearGolden.data(), linearVulkan.data(), TEST_WIDTH, TEST_HEIGHT,
                           false, parameters, true, true, meanError, &outMagmaRaw);
            std::unique_ptr<float[]> outMagma(outMagmaRaw);  // assumes FLIP::evaluate allocates with new[]
            Image::WriteImageRGB(outMagma.get(), TEST_WIDTH, TEST_HEIGHT, magmaVulkanPath);
            finalMean += meanError;
            backendCount++;
        }

        TestResult d3dData = test->Run(RHIBackend::kD3D12);
        Image::WriteImageData(d3dData.Data, d3dPath);
        std::vector<float> linearD3D(TEST_WIDTH * TEST_HEIGHT * 3);
        ConvertFLIP(d3dData.Data.Pixels.data(), linearD3D.data(), TEST_WIDTH, TEST_HEIGHT);

        {
            FLIP::Parameters parameters;
            float meanError;
            float* outMagmaRaw = nullptr;
            FLIP::evaluate(linearGolden.data(), linearD3D.data(), TEST_WIDTH, TEST_HEIGHT,
                           false, parameters, true, true, meanError, &outMagmaRaw);
            std::unique_ptr<float[]> outMagma(outMagmaRaw);
            Image::WriteImageRGB(outMagma.get(), TEST_WIDTH, TEST_HEIGHT, magmaD3DPath);
            finalMean += meanError;
            backendCount++;
        }

        json[test->Name()]["vkPath"] = StripDataPrefix(vulkanPath);
        json[test->Name()]["d3dPath"] = StripDataPrefix(d3dPath);
        json[test->Name()]["magmaVkPath"] = StripDataPrefix(magmaVulkanPath);
        json[test->Name()]["magmaD3D12Path"] = StripDataPrefix(magmaD3DPath);
    #endif

    #ifdef SERAPH_MAC
        TestResult metalData = test->Run(RHIBackend::kMetal);
        Image::WriteImageData(metalData.Data, metalPath);
        std::vector<float> linearMetal(TEST_WIDTH * TEST_HEIGHT * 3);
        ConvertFLIP(metalData.Data.Pixels.data(), linearMetal.data(), TEST_WIDTH, TEST_HEIGHT);

        {
            FLIP::Parameters parameters;
            float meanError;
            float* outMagmaRaw = nullptr;
            FLIP::evaluate(linearGolden.data(), linearMetal.data(), TEST_WIDTH, TEST_HEIGHT,
                           false, parameters, true, true, meanError, &outMagmaRaw);
            std::unique_ptr<float[]> outMagma(outMagmaRaw);
            Image::WriteImageRGB(outMagma.get(), TEST_WIDTH, TEST_HEIGHT, magmaMetalPath);
            finalMean += meanError;
            backendCount++;
        }

        json[test->Name()]["metalPath"] = StripDataPrefix(metalPath);
        json[test->Name()]["magmaMetalPath"] = StripDataPrefix(magmaMetalPath);
    #endif

        bool passed = (backendCount > 0) ? (finalMean / backendCount) < 0.02f : false;

        json[test->Name()]["goldenPath"] = StripDataPrefix(goldenPath);
        json[test->Name()]["result"] = passed;

        if (passed) testPassed++;

        delete test;
    }

    printf("-------------------------------------\n");
    SERAPH_INFO("TESTS PASSED: %d/%d", testPassed, testCount);
    printf("-------------------------------------\n");

    FileSystem::WriteJSON(json, "Data/TestReport.json");
    Context::Shutdown();
}
