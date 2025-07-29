//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:47:56
//

#include <KDShader/KDS_IncludeHandler.h>
#include <catch2/catch_test_macros.hpp>

const char* testIncludeShader = R"(
#include "steam_appid.txt"
#include ".gitkeep"
)";

TEST_CASE("include", "[KDS::IncludeHandler]") {
    KDS::ParseResult result;
    result.Includes.push_back(KDS::IncludeRecord{ "steam_appid.txt", 0 });
    result.Includes.push_back(KDS::IncludeRecord{ ".gitkeep", 1 });

    KDS::IncludeHandler handler;
    handler.AddIncludePath("data");

    KC::Array<KC::String> found;
    auto src = handler.ReplaceIncludes(result, testIncludeShader, found);

    auto lines = KC::StringUtil::ToLines(src);
    REQUIRE(lines[0] == "480");
    REQUIRE(lines[1] == "1");
}
