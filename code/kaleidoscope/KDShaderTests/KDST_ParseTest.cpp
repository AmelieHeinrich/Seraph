//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:06:19
//

#include <KDShader/KDS_Parser.h>
#include <catch2/catch_test_macros.hpp>

const char* testShader = R"(
    #include "test.hlsl"
    #include "shader.hlsl"

    #pragma vertex VS
    #pragma pixel PS
    #pragma compute CS
)";

TEST_CASE("parse simple", "[KDS::Parser]") {
    KDS::ParseResult result = KDS::Parser::ParseShaderSource(testShader);

    REQUIRE(result.Includes.size() > 0);
    REQUIRE(result.Includes[0].Path == "test.hlsl");
    REQUIRE(result.Includes[1].Path == "shader.hlsl");

    REQUIRE(result.EntryPoints.size() > 0);
    REQUIRE(result.EntryPoints[0].Name == "VS");
    REQUIRE(result.EntryPoints[0].Stage == KGPU::ShaderStage::kVertex);
    REQUIRE(result.EntryPoints[1].Name == "PS");
    REQUIRE(result.EntryPoints[1].Stage == KGPU::ShaderStage::kPixel);
    REQUIRE(result.EntryPoints[2].Name == "CS");
    REQUIRE(result.EntryPoints[2].Stage == KGPU::ShaderStage::kCompute);
}
