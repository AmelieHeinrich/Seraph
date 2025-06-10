//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-10 19:32:12
//

#include "DescriptorAllocator.h"

DescriptorAllocator::DescriptorAllocator(uint64 maxSlots)
    : mMaxSlots(maxSlots), mBitmapSize((maxSlots + 63) / 64)
{
    mFreeList.reserve(mMaxSlots);
    mBitmap.resize(mBitmapSize);

    for (uint i = 0; i < maxSlots; i++) {
        mFreeList.push_back(i);
    }
}

DescriptorAllocator::~DescriptorAllocator()
{
    mBitmap.clear();
    mFreeList.clear();
}

uint32 DescriptorAllocator::Allocate()
{
    if (!mFreeList.empty()) {
        uint index = mFreeList.back();
        mFreeList.pop_back();
        SetBit(index);
        return index;
    }
    return INVALID_DESCRIPTOR;
}

void DescriptorAllocator::Free(uint index)
{
    if (!TestBit(index)) return;
    ClearBit(index);
    mFreeList.push_back(index);
}

void DescriptorAllocator::SetBit(uint index)
{
    mBitmap[index / 64] |= (1ull << (index % 64));
}

void DescriptorAllocator::ClearBit(uint index)
{
    mBitmap[index / 64] &= ~(1ull << (index % 64));
}

bool DescriptorAllocator::TestBit(uint index)
{
    return (mBitmap[index / 64] >> (index % 64)) & 1ull;
}
