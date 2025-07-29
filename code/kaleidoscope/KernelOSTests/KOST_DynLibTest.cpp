//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:36:30
//


#include <KernelOS/KOS_DynLib.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Dyn Lib Open", "[dynlib]") {
    KOS::DynLib lib("opengl32.dll");
    REQUIRE(lib.Valid());
}

TEST_CASE("Dyn Lib Load", "[dynlib]") {
    KOS::DynLib lib("opengl32.dll");
    REQUIRE(lib.GetFunction("glDrawArrays"));
}
