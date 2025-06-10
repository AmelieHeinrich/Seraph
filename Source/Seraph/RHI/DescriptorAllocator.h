//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-10 19:29:43
//

#pragma once

#include <Core/Types.h>

constexpr uint INVALID_DESCRIPTOR = 0xFFFFFFFF;

class DescriptorAllocator
{
public:
    DescriptorAllocator(uint64 maxSlots);
    ~DescriptorAllocator();

    uint32 Allocate();
    void Free(uint index);
private:
    void SetBit(uint index);
    void ClearBit(uint index);
    bool TestBit(uint index);

    uint64 mMaxSlots;
    uint64 mBitmapSize;
    Array<uint64> mBitmap;
    Array<uint32> mFreeList;
};
