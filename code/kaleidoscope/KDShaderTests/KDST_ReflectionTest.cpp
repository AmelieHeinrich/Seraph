//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 22:07:08
//

#include <KDShader/KDS_Compiler.h>
#include <KDShader/KDS_ReflectionEngine.h>
#include <catch2/catch_test_macros.hpp>

const char* testReflect = R"(
    struct foo { float4 data; };

    ConstantBuffer<foo> push : register(b0);

    [numthreads(8, 8, 1)]
    void CSMain(uint3 dtid : SV_DispatchThreadID) {
        RWTexture2D<float> tex = ResourceDescriptorHeap[0];
        tex[dtid.xy] = push.data.x * push.data.y;
    }
)";

TEST_CASE("include", "[KDS::ReflectionEngine]") {
    KGPU::ShaderBytecodeType type;
#if KD_WINDOWS
    type = KGPU::ShaderBytecodeType::kDXIL;
#endif
    KDS::ICompiler* compiler = KDS::ICompiler::Create(type);
    KGPU::ShaderModule module = compiler->Compile(testReflect, "CSMain", KGPU::ShaderStage::kCompute);
    KC_DELETE(compiler);

    KDS::IReflectionEngine* engine = KDS::IReflectionEngine::Create(type);
    KDS::ReflectionResult result = engine->Reflect(module);
    REQUIRE(result.PushConstantSize == sizeof(KGPU::uint4));
    REQUIRE(result.ThreadsPerGroup.x == 8);
    REQUIRE(result.ThreadsPerGroup.y == 8);
    REQUIRE(result.ThreadsPerGroup.z == 1);
    KC_DELETE(engine);
}
