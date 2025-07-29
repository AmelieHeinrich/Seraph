//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:10:31
//

#pragma once

#include "KC_Common.h"
#include "KC_Array.h"

namespace KC
{
    constexpr int FREE_LIST_INVALID_ALLOC = -1;

    class FreeList
    {
    public:
        FreeList(uint64 maxSlots);
        ~FreeList();

        int32 Allocate();
        void Free(int32 index);
    private:
        void SetBit(int32 index);
        void ClearBit(int32 index);
        bool TestBit(int32 index);

        uint64 mMaxSlots;
        uint64 mBitmapSize;
        Array<uint64> mBitmap;
        Array<uint32> mFreeList;
    };
}
