//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:13:17
//

#include "KC_FreeList.h"

namespace KC {
    FreeList::FreeList(uint64 maxSlots)
        : mMaxSlots(maxSlots), mBitmapSize((maxSlots + 63) / 64)
    {
        mFreeList.reserve(mMaxSlots);
        mBitmap.resize(mBitmapSize, 0);  // RESIZE and initialize to zeros!

        for (int32 i = 0; i < maxSlots; i++) {
            mFreeList.push_back(i);
        }
    }

    FreeList::~FreeList()
    {
        mBitmap.clear();
        mFreeList.clear();
    }

    int32 FreeList::Allocate()
    {
        if (!mFreeList.empty()) {
            int32 index = mFreeList.back();
            mFreeList.pop_back();
            SetBit(index);
            return index;
        }
        return FREE_LIST_INVALID_ALLOC;
    }

    void FreeList::Free(int32 index)
    {
        if (index < 0 || static_cast<uint64>(index) >= mMaxSlots) return; // bounds check
        if (!TestBit(index)) return;  // already free
        ClearBit(index);
        mFreeList.push_back(index);
    }

    void FreeList::SetBit(int32 index)
    {
        mBitmap[index / 64] |= (1ull << (index % 64));
    }

    void FreeList::ClearBit(int32 index)
    {
        mBitmap[index / 64] &= ~(1ull << (index % 64));
    }

    bool FreeList::TestBit(int32 index)
    {
        return (mBitmap[index / 64] >> (index % 64)) & 1ull;
    }
}
