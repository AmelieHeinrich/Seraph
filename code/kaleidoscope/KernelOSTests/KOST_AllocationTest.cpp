//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:20:07
//

#include <KernelOS/KOS_DirectAllocation.h>
#include <catch2/catch_test_macros.hpp>

class MyClass
{
public:
    int data = 0;
};

TEST_CASE("malloc", "[memory]") {
    uint8* mem = (uint8*)KOS::AllocateMem(sizeof(uint8) * 2);
    REQUIRE(mem != nullptr);
    *mem = 0;
    REQUIRE(*mem == 0);
    KOS::FreeMem(mem);
}
