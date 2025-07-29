//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:41:39
//

#include <KernelCore/KC_OffsetAllocator.h>
#include <catch2/catch_test_macros.hpp>

namespace KC
{
    extern uint32 UIntToFloatRoundUp(uint32 size);
    extern uint32 UIntToFloatRoundDown(uint32 size);
    extern uint32 FloatToUInt(uint32 floatValue);
}

using namespace KC;

TEST_CASE("numbers", "[SmallFloat]")
{
    SECTION("UIntToFloat")
    {
        // Denorms, exp=1 and exp=2 + mantissa = 0 are all precise.
        // NOTE: Assuming 8 value (3 bit) mantissa.
        // If this test fails, please change this assumption!
        uint32 preciseNumberCount = 17;
        for (uint32 i = 0; i < preciseNumberCount; i++)
        {
            uint32 roundUp = UIntToFloatRoundUp(i);
            uint32 roundDown = UIntToFloatRoundDown(i);
            REQUIRE(i == roundUp);
            REQUIRE(i == roundDown);
        }
        
        // Test some random picked numbers
        struct NumberFloatUpDown
        {
            uint32 number;
            uint32 up;
            uint32 down;
        };
        
        NumberFloatUpDown testData[] = {
            {.number = 17, .up = 17, .down = 16},
            {.number = 118, .up = 39, .down = 38},
            {.number = 1024, .up = 64, .down = 64},
            {.number = 65536, .up = 112, .down = 112},
            {.number = 529445, .up = 137, .down = 136},
            {.number = 1048575, .up = 144, .down = 143},
        };
        
        for (uint32 i = 0; i < sizeof(testData) / sizeof(NumberFloatUpDown); i++)
        {
            NumberFloatUpDown v = testData[i];
            uint32 roundUp = UIntToFloatRoundUp(v.number);
            uint32 roundDown = UIntToFloatRoundDown(v.number);
            REQUIRE(roundUp == v.up);
            REQUIRE(roundDown == v.down);
        }
    }
    
    SECTION("FloatToUInt")
    {
        // Denorms, exp=1 and exp=2 + mantissa = 0 are all precise.
        // NOTE: Assuming 8 value (3 bit) mantissa.
        // If this test fails, please change this assumption!
        uint32 preciseNumberCount = 17;
        for (uint32 i = 0; i < preciseNumberCount; i++)
        {
            uint32 v = FloatToUInt(i);
            REQUIRE(i == v);
        }
        
        // Test that float->uint->float conversion is precise for all numbers
        // NOTE: Test values < 240. 240->4G = overflows 32 bit integer
        for (uint32 i = 0; i < 240; i++)
        {
            uint32 v = FloatToUInt(i);
            uint32 roundUp = UIntToFloatRoundUp(v);
            uint32 roundDown = UIntToFloatRoundDown(v);
            REQUIRE(i == roundUp);
            REQUIRE(i == roundDown);
        }
    }
}

TEST_CASE("basic", "[KC::OffsetAllocator]")
{
    OffsetAllocator allocator(1024 * 1024 * 256);
    OffsetAllocator::Allocation a = allocator.Allocate(1337);
    uint32 offset = a.Offset;
    REQUIRE(offset == 0);
    allocator.Free(a);
}

TEST_CASE("allocate", "[KC::OffsetAllocator]")
{
    OffsetAllocator allocator(1024 * 1024 * 256);
    SECTION("simple")
    {
        // Free merges neighbor empty nodes. Next allocation should also have offset = 0
        OffsetAllocator::Allocation a = allocator.Allocate(0);
        REQUIRE(a.Offset == 0);
        
        OffsetAllocator::Allocation b = allocator.Allocate(1);
        REQUIRE(b.Offset == 0);
        OffsetAllocator::Allocation c = allocator.Allocate(123);
        REQUIRE(c.Offset == 1);
        OffsetAllocator::Allocation d = allocator.Allocate(1234);
        REQUIRE(d.Offset == 124);
        allocator.Free(a);
        allocator.Free(b);
        allocator.Free(c);
        allocator.Free(d);
        
        // End: Validate that allocator has no fragmentation left. Should be 100% clean.
        OffsetAllocator::Allocation validateAll = allocator.Allocate(1024 * 1024 * 256);
        REQUIRE(validateAll.Offset == 0);
        allocator.Free(validateAll);
    }

    SECTION("merge trivial")
    {
        // Free merges neighbor empty nodes. Next allocation should also have offset = 0
        OffsetAllocator::Allocation a = allocator.Allocate(1337);
        REQUIRE(a.Offset == 0);
        allocator.Free(a);
        
        OffsetAllocator::Allocation b = allocator.Allocate(1337);
        REQUIRE(b.Offset == 0);
        allocator.Free(b);
        
        // End: Validate that allocator has no fragmentation left. Should be 100% clean.
        OffsetAllocator::Allocation validateAll = allocator.Allocate(1024 * 1024 * 256);
        REQUIRE(validateAll.Offset == 0);
        allocator.Free(validateAll);
    }
    
    SECTION("reuse trivial")
    {
        // Allocator should reuse node freed by A since the allocation C fits in the same bin (using pow2 size to be sure)
        OffsetAllocator::Allocation a = allocator.Allocate(1024);
        REQUIRE(a.Offset == 0);
        OffsetAllocator::Allocation b = allocator.Allocate(3456);
        REQUIRE(b.Offset == 1024);
        allocator.Free(a);
        
        OffsetAllocator::Allocation c = allocator.Allocate(1024);
        REQUIRE(c.Offset == 0);
        allocator.Free(c);
        allocator.Free(b);
        
        // End: Validate that allocator has no fragmentation left. Should be 100% clean.
        OffsetAllocator::Allocation validateAll = allocator.Allocate(1024 * 1024 * 256);
        REQUIRE(validateAll.Offset == 0);
        allocator.Free(validateAll);
    }

    SECTION("reuse complex")
    {
        // Allocator should not reuse node freed by A since the allocation C doesn't fits in the same bin
        // However node D and E fit there and should reuse node from A
        OffsetAllocator::Allocation a = allocator.Allocate(1024);
        REQUIRE(a.Offset == 0);
        OffsetAllocator::Allocation b = allocator.Allocate(3456);
        REQUIRE(b.Offset == 1024);
        allocator.Free(a);
        
        OffsetAllocator::Allocation c = allocator.Allocate(2345);
        REQUIRE(c.Offset == 1024 + 3456);
        OffsetAllocator::Allocation d = allocator.Allocate(456);
        REQUIRE(d.Offset == 0);
        OffsetAllocator::Allocation e = allocator.Allocate(512);
        REQUIRE(e.Offset == 456);
        allocator.Free(c);
        allocator.Free(d);
        allocator.Free(b);
        allocator.Free(e);
        
        // End: Validate that allocator has no fragmentation left. Should be 100% clean.
        OffsetAllocator::Allocation validateAll = allocator.Allocate(1024 * 1024 * 256);
        REQUIRE(validateAll.Offset == 0);
        allocator.Free(validateAll);
    }
}
