//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:19:04
//

#include <KernelOS/KOS_Common.h>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Align") {
    REQUIRE(KOS::Align<uint>(1, 16) == 16);
}
