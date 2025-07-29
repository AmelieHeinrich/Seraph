//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:23:10
//

/*
    Credit to Sebastian Aaltonen : https://github.com/sebbbi/OffsetAllocator/tree/main

    MIT License

    Copyright (c) 2023 Sebastian Aaltonen

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#pragma once

#include "KC_Common.h"

namespace KC
{
    static constexpr uint32 OFFSET_ALLOCATOR_NUM_TOP_BINS = 32;
    static constexpr uint32 OFFSET_ALLOCATOR_BINS_PER_LEAF = 8;
    static constexpr uint32 OFFSET_ALLOCATOR_TOP_BINS_INDEX_SHIFT = 3;
    static constexpr uint32 OFFSET_ALLOCATOR_LEAF_BINS_INDEX_MASK = 0x7;
    static constexpr uint32 OFFSET_ALLOCATOR_NUM_LEAF_BINS = OFFSET_ALLOCATOR_NUM_TOP_BINS * OFFSET_ALLOCATOR_BINS_PER_LEAF;
    static constexpr uint32 OFFSET_ALLOCATOR_INVALID = 0xFFFFFFFF;

    class OffsetAllocator
    {
    public:
        struct Allocation
        {
            uint Offset = OFFSET_ALLOCATOR_INVALID;
            uint Metadata = OFFSET_ALLOCATOR_INVALID;
        };

        OffsetAllocator(uint size);
        ~OffsetAllocator();

        void Reset();
        Allocation Allocate(uint32 size);
        void Free(Allocation allocation);

    private:
        uint InsertNodeIntoBin(uint size, uint dataOffset);
        void RemoveNodeFromBin(uint nodeIndex);

        struct Node
        {
            uint DataOffset = 0;
            uint DataSize = 0;
            uint BinListPrev = OFFSET_ALLOCATOR_INVALID;
            uint BinListNext = OFFSET_ALLOCATOR_INVALID;
            uint NeighborPrev = OFFSET_ALLOCATOR_INVALID;
            uint NeighborNext = OFFSET_ALLOCATOR_INVALID;
            bool Used = false;
        };

        uint mSize;
        uint mFreeStorage;
        uint mMaxAllocs;
        
        uint mUsedBinsTop;
        uint8 mUsedBins[OFFSET_ALLOCATOR_NUM_TOP_BINS];
        uint mBinIndices[OFFSET_ALLOCATOR_NUM_LEAF_BINS];

        Node* mNodes = nullptr;
        uint* mFreeNodes = nullptr;
        uint mFreeOffset;
    };
}
