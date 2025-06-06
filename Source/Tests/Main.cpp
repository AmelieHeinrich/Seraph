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
    nlohmann::json json;

    auto StripDataPrefix = [](const std::string& path) -> std::string {
        const std::string prefix = "Data/";
        if (path.rfind(prefix, 0) == 0) { // starts with "Data/"
            return path.substr(prefix.length());
        }
        return path;
    };

    for (auto* test : tests) {
        std::string goldenPath = "Data/Tests/Golden/" + std::string(test->Name()) + "Golden" + ".png";
        std::string magmaVulkanPath = "Data/Tests/" + std::string(test->Name()) + "MagmaVulkan" + ".png";
        std::string magmaD3DPath = "Data/Tests/" + std::string(test->Name()) + "MagmaD3D12" + ".png";
        std::string vulkanPath = "Data/Tests/" + std::string(test->Name()) + std::string("Vulkan") + ".png";
        std::string d3dPath = "Data/Tests/" + std::string(test->Name()) + std::string("D3D12") + ".png";

        ImageData golden = Image::LoadImageData(goldenPath);
        TestResult vulkanData = test->Run(RHIBackend::kVulkan);
        TestResult d3dData = test->Run(RHIBackend::kD3D12);
        Image::WriteImageData(vulkanData.Data, vulkanPath);
        Image::WriteImageData(d3dData.Data, d3dPath);

        vulkanData.Data.Pixels.clear();
        d3dData.Data.Pixels.clear();

        float* linearVulkan = new float[TEST_WIDTH * TEST_HEIGHT * 3];
        float* linearD3D = new float[TEST_WIDTH * TEST_HEIGHT * 3];
        float* linearGolden = new float[TEST_WIDTH * TEST_HEIGHT * 3];
        ConvertFLIP(vulkanData.Data.Pixels.data(), linearVulkan, TEST_WIDTH, TEST_HEIGHT);
        ConvertFLIP(d3dData.Data.Pixels.data(), linearD3D, TEST_WIDTH, TEST_HEIGHT);
        ConvertFLIP(golden.Pixels.data(), linearGolden, TEST_WIDTH, TEST_HEIGHT);

        float finalMean = 0.0f;
        {
            FLIP::Parameters parameters;
            float meanError;
            float* outMagma;
            FLIP::evaluate(linearGolden, linearVulkan, TEST_WIDTH, TEST_HEIGHT, false, parameters, true, true, meanError, &outMagma);
            Image::WriteImageRGB(outMagma, TEST_WIDTH, TEST_HEIGHT, magmaVulkanPath);
            delete outMagma;

            finalMean += meanError;
        }
        {
            FLIP::Parameters parameters;
            float meanError;
            float* outMagma;
            FLIP::evaluate(linearGolden, linearD3D, TEST_WIDTH, TEST_HEIGHT, false, parameters, true, true, meanError, &outMagma);
            Image::WriteImageRGB(outMagma, TEST_WIDTH, TEST_HEIGHT, magmaD3DPath);
            delete outMagma;

            finalMean += meanError;
        }
        finalMean /= 2;

        json[test->Name()]["vkPath"] = StripDataPrefix(vulkanPath);
        json[test->Name()]["d3dPath"] = StripDataPrefix(d3dPath);
        json[test->Name()]["magmaVkPath"] = StripDataPrefix(magmaVulkanPath);
        json[test->Name()]["magmaD3D12Path"] = StripDataPrefix(magmaD3DPath);
        json[test->Name()]["goldenPath"] = StripDataPrefix(goldenPath);
        json[test->Name()]["result"] = finalMean < 0.02f;

        delete[] linearGolden;
        delete[] linearVulkan;
        delete[] linearD3D;
    }

    FileSystem::WriteJSON(json, "Data/TestReport.json");
    Context::Shutdown();
}
