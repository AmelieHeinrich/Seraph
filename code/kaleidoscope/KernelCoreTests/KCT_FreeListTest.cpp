//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 19:40:32
//

#include <KernelCore/KC_FreeList.h>
#include <catch2/catch_test_macros.hpp>

using namespace KC;

TEST_CASE("FreeList basic allocation and free", "[KC::FreeList]") {
    const uint64 maxSlots = 64;
    FreeList freelist(maxSlots);

    SECTION("Allocate all slots") {
        Array<int32> allocated;
        for (uint64 i = 0; i < maxSlots; ++i) {
            int32 idx = freelist.Allocate();
            REQUIRE(idx != FREE_LIST_INVALID_ALLOC);
            allocated.push_back(idx);
        }

        // Should be exhausted now
        REQUIRE(freelist.Allocate() == FREE_LIST_INVALID_ALLOC);

        // Free half and allocate again
        for (uint64 i = 0; i < maxSlots / 2; ++i) {
            freelist.Free(allocated[i]);
        }
        for (uint64 i = 0; i < maxSlots / 2; ++i) {
            int32 idx = freelist.Allocate();
            REQUIRE(idx != FREE_LIST_INVALID_ALLOC);
        }
        // Exhausted again
        REQUIRE(freelist.Allocate() == FREE_LIST_INVALID_ALLOC);
    }

    SECTION("Double free should not break") {
        int32 idx = freelist.Allocate();
        REQUIRE(idx != FREE_LIST_INVALID_ALLOC);

        freelist.Free(idx);
        freelist.Free(idx); // freeing twice

        int32 newIdx = freelist.Allocate();
        REQUIRE(newIdx != FREE_LIST_INVALID_ALLOC);
    }

    SECTION("Freeing invalid index does not crash") {
        freelist.Free(-1);
        freelist.Free(static_cast<int32>(maxSlots + 10));
    }
}

TEST_CASE("FreeList bit operations", "[KC::FreeList][Private]") {
    const uint64 maxSlots = 128;
    FreeList freelist(maxSlots);

    // Directly test private bit operations via friend test or exposed for test (if possible)
    // For now we just allocate/free to indirectly test bitmap.

    int32 idx1 = freelist.Allocate();
    REQUIRE(idx1 >= 0);

    int32 idx2 = freelist.Allocate();
    REQUIRE(idx2 >= 0);
    REQUIRE(idx1 != idx2);

    freelist.Free(idx1);
    freelist.Free(idx2);

    int32 idx3 = freelist.Allocate();
    int32 idx4 = freelist.Allocate();
    REQUIRE((idx3 == idx1 || idx3 == idx2));
    REQUIRE((idx4 == idx1 || idx4 == idx2));
}
