//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 21:37:24
//

#include <KDShader/KDS_Compiler.h>
#include <catch2/catch_test_macros.hpp>

const char* testCompute = R"(
    [numthreads(8, 8, 1)]
    void CSMain(uint3 dtid : SV_DispatchThreadID)
    {
        RWTexture2D<float> tex = ResourceDescriptorHeap[0];
        tex[dtid.xy] = 1.0f;
    }
)";

TEST_CASE("include", "[KDS::Compiler]") {
#if KD_WINDOWS
    KGPU::ShaderBytecodeType type = KGPU::ShaderBytecodeType::kDXIL;
#endif

    KDS::ICompiler* compiler = KDS::ICompiler::Create(type);
    REQUIRE(compiler != nullptr);

    KGPU::ShaderModule module = compiler->Compile(testCompute, "CSMain", KGPU::ShaderStage::kCompute);
    REQUIRE(module.Data.size() > 0);
    REQUIRE(module.Entry == "CSMain");
    REQUIRE(module.Stage == KGPU::ShaderStage::kCompute);
    REQUIRE(module.Language == KGPU::ShaderLanguage::kHLSL);

    KC_DELETE(compiler);
}
