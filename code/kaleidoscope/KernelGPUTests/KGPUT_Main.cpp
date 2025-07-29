//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-26 00:23:26
//

#include <flip.h>
#include <json.hpp>
#include <stb_image_write.h>

#include <KernelCore/KC_Context.h>
#include <KDAsset/KDA_TextureLoader.h>

#include "KGPUT_Test.h"

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

void ConvertOutput(const float* linearRGB, uint8* rgba8, int width, int height)
{
    for (int i = 0; i < width * height; ++i) {
        rgba8[i * 4 + 0] = linearRGB[i * 3 + 0] / 255.0f;
        rgba8[i * 4 + 1] = linearRGB[i * 3 + 1] / 255.0f;
        rgba8[i * 4 + 2] = linearRGB[i * 3 + 2] / 255.0f;
        rgba8[i * 4 + 3] = 255;
    }
}

KD_MAIN
{
    KC::ScopedContext ctx;
    CODE_BLOCK("Run Tests") {
        auto& tests = KGPUT::GetTests();
        nlohmann::json json;

        auto StripDataPrefix = [](const KC::String& path) -> std::string {
            const KC::String prefix = "data/kd/";
            if (path.rfind(prefix, 0) == 0) { // starts with "Data/"
                return path.substr(prefix.length());
            }
            return path;
        };

        int testCount = tests.size();
        int testPassed = 0;
        for (auto* test : tests) {
            KC::String goldenPath = "data/kd/tests/golden/" + KC::String(test->Name()) + "Golden.png";
            KC::String magmaPath = "data/kd/tests/" + KC::String(test->Name()) + "Magma.png";
            KC::String outputPath = "data/kd/tests/" + KC::String(test->Name()) + ".png";

            KDA::Texture golden = KDA::TextureLoader::LoadFromFile(goldenPath);
            KC::Array<float> linearGolden(KGPUT::TEST_WIDTH * KGPUT::TEST_HEIGHT * 3);
            ConvertFLIP(golden.Bytes.data(), linearGolden.data(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT);

            auto testResult = test->Run();
            stbi_write_png(outputPath.c_str(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT, 4, testResult.data(), KGPUT::TEST_WIDTH * 4);
            KC::Array<float> linearOutput(KGPUT::TEST_WIDTH * KGPUT::TEST_HEIGHT * 3);
            ConvertFLIP(testResult.data(), linearOutput.data(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT);
        
            FLIP::Parameters parameters;
            float meanError = 0.0f;
            float* outMagma = nullptr;
            FLIP::evaluate(linearGolden.data(), linearOutput.data(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT, false, parameters, true, true, meanError, &outMagma);
            KC::Array<uint8> magmaOutput(KGPUT::TEST_WIDTH * KGPUT::TEST_HEIGHT * 4);
            ConvertOutput(outMagma, magmaOutput.data(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT);
            stbi_write_png(magmaPath.c_str(), KGPUT::TEST_WIDTH, KGPUT::TEST_HEIGHT, 4, magmaOutput.data(), KGPUT::TEST_WIDTH * 4);
            delete[] outMagma;

            bool result = meanError < 0.02f;
            json[test->Name()]["goldenPath"] = StripDataPrefix(goldenPath);
            json[test->Name()]["outputPath"] = StripDataPrefix(outputPath);
            json[test->Name()]["magmaPath"] = StripDataPrefix(magmaPath);
            json[test->Name()]["result"] = result;
            testPassed += result;

            KD_INFO("> %s : %s", test->Name(), result ? "PASS" : "FAIL");
        }

        KD_INFO("-------------------------------------");
        KD_INFO("TESTS PASSED: %d/%d", testPassed, testCount);
        KD_INFO("-------------------------------------");
    
        KC::String string = json.dump(4);
        KC::FileStream out("data/kd/test_report.json", KOS::FileOpenMode::kWrite | KOS::FileOpenMode::kOverwrite);
        out.Write(string);
        out.Close();
    }
    return 0;
}
