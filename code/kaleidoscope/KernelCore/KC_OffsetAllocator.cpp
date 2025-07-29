//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:30:15
//

#include "KC_OffsetAllocator.h"
#include "KC_Context.h"

#include <KernelOS/KOS_DirectAllocation.h>

#ifdef _MSC_VER
    #include <intrin.h>
#endif

namespace KC {
    static constexpr uint32 MANTISSA_BITS = 3;
    static constexpr uint32 MANTISSA_VALUE = 1 << MANTISSA_BITS;
    static constexpr uint32 MANTISSA_MASK = MANTISSA_VALUE - 1;

    inline uint lzcnt_nonzero(uint v)
    {
#ifdef _MSC_VER
        unsigned long retValue;
        _BitScanReverse(&retValue, v);
        return 31 - retValue;
#else
        return __builtin_clz(v);
#endif
    }

    inline uint tzcnt_nonzero(uint v)
    {
#ifdef _MSC_VER
        unsigned long retValue;
        _BitScanForward(&retValue, v);
        return retValue;
#else
        return __builtin_ctz(v);
#endif
    }

    uint32 UIntToFloatRoundUp(uint32 size)
    {
        uint32 exp = 0;
        uint32 mantissa = 0;
        
        if (size < MANTISSA_VALUE) {
            mantissa = size;
        } else {
            uint32 leadingZeros = lzcnt_nonzero(size);
            uint32 highestSetBit = 31 - leadingZeros;
            
            uint32 mantissaStartBit = highestSetBit - MANTISSA_BITS;
            exp = mantissaStartBit + 1;
            mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;
            
            uint32 lowBitsMask = (1 << mantissaStartBit) - 1;
            
            if ((size & lowBitsMask) != 0)
                mantissa++;
        }
        return (exp << MANTISSA_BITS) + mantissa;
    }

    uint32 UIntToFloatRoundDown(uint32 size)
    {
        uint32 exp = 0;
        uint32 mantissa = 0;
        
        if (size < MANTISSA_VALUE) {
            mantissa = size;
        } else {
            uint32 leadingZeros = lzcnt_nonzero(size);
            uint32 highestSetBit = 31 - leadingZeros;
            
            uint32 mantissaStartBit = highestSetBit - MANTISSA_BITS;
            exp = mantissaStartBit + 1;
            mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;
        }
        return (exp << MANTISSA_BITS) | mantissa;
    }

    uint32 FloatToUInt(uint32 floatValue)
    {
        uint32 exponent = floatValue >> MANTISSA_BITS;
        uint32 mantissa = floatValue & MANTISSA_MASK;
        if (exponent == 0) {
            return mantissa;
        } else {
            return (mantissa | MANTISSA_VALUE) << (exponent - 1);
        }
    }

    uint32 FindLowestSetBitAfter(uint32 bitMask, uint32 startBitIndex)
    {
        uint32 maskBeforeStartIndex = (1 << startBitIndex) - 1;
        uint32 maskAfterStartIndex = ~maskBeforeStartIndex;
        uint32 bitsAfter = bitMask & maskAfterStartIndex;
        if (bitsAfter == 0) return OFFSET_ALLOCATOR_INVALID;
        return tzcnt_nonzero(bitsAfter);
    }

    OffsetAllocator::OffsetAllocator(uint size)
        : mSize(size), mMaxAllocs(128 * 1024)
    {
        Reset();
    }

    OffsetAllocator::~OffsetAllocator()
    {
        KC_DELETE_ARRAY(mNodes, mMaxAllocs);
        KC_DELETE_ARRAY(mFreeNodes, mMaxAllocs);
    }

    void OffsetAllocator::Reset()
    {
        mFreeStorage = 0;
        mUsedBinsTop = 0;
        mFreeOffset = mMaxAllocs - 1;

        for (uint32 i = 0 ; i < OFFSET_ALLOCATOR_NUM_TOP_BINS; i++)
            mUsedBins[i] = 0;
        for (uint32 i = 0 ; i < OFFSET_ALLOCATOR_NUM_LEAF_BINS; i++)
            mBinIndices[i] = OFFSET_ALLOCATOR_INVALID;

        if (mNodes) KC_DELETE_ARRAY(mNodes, mMaxAllocs);
        if (mFreeNodes) KC_DELETE_ARRAY(mFreeNodes, mMaxAllocs);

        mNodes = KC_NEW_ARRAY(Node, mMaxAllocs);
        mFreeNodes = KC_NEW_ARRAY(uint, mMaxAllocs);
    
        for (uint32 i = 0; i < mMaxAllocs; i++) {
            mFreeNodes[i] = mMaxAllocs - 1 - i;
        }
        InsertNodeIntoBin(mSize, 0);
    }

    OffsetAllocator::Allocation OffsetAllocator::Allocate(uint size)
    {
        if (mFreeOffset == 0) {
            return {
                .Offset = OFFSET_ALLOCATOR_INVALID,
                .Metadata = OFFSET_ALLOCATOR_INVALID
            };
        }

        uint32 minBinIndex = UIntToFloatRoundUp(size);
        
        uint32 minTopBinIndex = minBinIndex >> OFFSET_ALLOCATOR_TOP_BINS_INDEX_SHIFT;
        uint32 minLeafBinIndex = minBinIndex & OFFSET_ALLOCATOR_LEAF_BINS_INDEX_MASK;
        
        uint32 topBinIndex = minTopBinIndex;
        uint32 leafBinIndex = OFFSET_ALLOCATOR_INVALID;
        
        if (mUsedBinsTop & (1 << topBinIndex)) {
            leafBinIndex = FindLowestSetBitAfter(mUsedBins[topBinIndex], minLeafBinIndex);
        }

        if (leafBinIndex == OFFSET_ALLOCATOR_INVALID) {
            topBinIndex = FindLowestSetBitAfter(mUsedBinsTop, minTopBinIndex + 1);
            if (topBinIndex == OFFSET_ALLOCATOR_INVALID) {
                return {
                    .Offset = OFFSET_ALLOCATOR_INVALID,
                    .Metadata = OFFSET_ALLOCATOR_INVALID
                };
            }
            leafBinIndex = tzcnt_nonzero(mUsedBins[topBinIndex]);
        }

        uint32 binIndex = (topBinIndex << OFFSET_ALLOCATOR_TOP_BINS_INDEX_SHIFT) | leafBinIndex;

        uint32 nodeIndex = mBinIndices[binIndex];
        Node& node = mNodes[nodeIndex];

        uint32 nodeTotalSize = node.DataSize;
        node.DataSize = size;
        node.Used = true;

        mBinIndices[binIndex] = node.BinListNext;
        if (node.BinListNext != OFFSET_ALLOCATOR_INVALID) mNodes[node.BinListNext].BinListPrev = OFFSET_ALLOCATOR_INVALID;
        mFreeStorage -= nodeTotalSize;
    
        if (mBinIndices[binIndex] == OFFSET_ALLOCATOR_INVALID) {
            mUsedBins[topBinIndex] &= ~(1 << leafBinIndex);
            if (mUsedBins[topBinIndex] == 0) {
                mUsedBinsTop &= ~(1 << topBinIndex);
            }
        }

        uint32 reminderSize = nodeTotalSize - size;
        if (reminderSize > 0) {
            uint32 newNodeIndex = InsertNodeIntoBin(reminderSize, node.DataOffset + size);
            
            if (node.NeighborNext != OFFSET_ALLOCATOR_INVALID) mNodes[node.NeighborNext].NeighborPrev = newNodeIndex;
            mNodes[newNodeIndex].NeighborPrev = nodeIndex;
            mNodes[newNodeIndex].NeighborNext = node.NeighborNext;
            node.NeighborNext = newNodeIndex;
        }

        return {
            .Offset = node.DataOffset,
            .Metadata = nodeIndex
        };
    }

    void OffsetAllocator::Free(Allocation allocation)
    {
        KD_ASSERT_EQ(allocation.Metadata != OFFSET_ALLOCATOR_INVALID, "Allocation passed to OffsetAllocator::Free is invalid!");
        if (!mNodes) return;

        uint nodeIndex = allocation.Metadata;
        Node& node = mNodes[nodeIndex];
        KD_ASSERT_EQ(node.Used == true, "");
    
        uint offset = node.DataOffset;
        uint size = node.DataSize;
        if ((node.NeighborPrev != OFFSET_ALLOCATOR_INVALID) && (mNodes[node.NeighborPrev].Used == false)) {
            Node& prevNode = mNodes[node.NeighborPrev];
            offset = prevNode.DataOffset;
            size += prevNode.DataSize;

            RemoveNodeFromBin(node.NeighborPrev);

            KD_ASSERT_EQ(prevNode.NeighborNext == nodeIndex, "");
            node.NeighborPrev = prevNode.NeighborPrev;
        }
        if ((node.NeighborNext != OFFSET_ALLOCATOR_INVALID) && (mNodes[node.NeighborNext].Used == false)) {
            Node& nextNode = mNodes[node.NeighborNext];
            size += nextNode.DataSize;

            RemoveNodeFromBin(node.NeighborNext);

            KD_ASSERT_EQ(nextNode.NeighborPrev == nodeIndex, "");
            node.NeighborNext = nextNode.NeighborNext;
        }

        uint32 neighborNext = node.NeighborNext;
        uint32 neighborPrev = node.NeighborPrev;
    
        mFreeNodes[++mFreeOffset] = nodeIndex;

        uint32 combinedNodeIndex = InsertNodeIntoBin(size, offset);
        if (neighborNext != OFFSET_ALLOCATOR_INVALID) {
            mNodes[combinedNodeIndex].NeighborNext = neighborNext;
            mNodes[neighborNext].NeighborPrev = combinedNodeIndex;
        }
        if (neighborPrev != OFFSET_ALLOCATOR_INVALID) {
            mNodes[combinedNodeIndex].NeighborPrev = neighborPrev;
            mNodes[neighborPrev].NeighborNext = combinedNodeIndex;
        }
    }

    uint OffsetAllocator::InsertNodeIntoBin(uint size, uint dataOffset)
    {
        uint32 binIndex = UIntToFloatRoundDown(size);
        
        uint32 topBinIndex = binIndex >> OFFSET_ALLOCATOR_TOP_BINS_INDEX_SHIFT;
        uint32 leafBinIndex = binIndex & OFFSET_ALLOCATOR_LEAF_BINS_INDEX_MASK;
        if (mBinIndices[binIndex] == OFFSET_ALLOCATOR_INVALID) {
            mUsedBins[topBinIndex] |= 1 << leafBinIndex;
            mUsedBinsTop |= 1 << topBinIndex;
        }
        
        uint32 topNodeIndex = mBinIndices[binIndex];
        uint32 nodeIndex = mFreeNodes[mFreeOffset--];
        mNodes[nodeIndex] = { .DataOffset = dataOffset, .DataSize = size, .BinListNext = topNodeIndex};
        if (topNodeIndex != OFFSET_ALLOCATOR_INVALID) mNodes[topNodeIndex].BinListPrev = nodeIndex;
        mBinIndices[binIndex] = nodeIndex;
        mFreeStorage += size;

        return nodeIndex;
    }

    void OffsetAllocator::RemoveNodeFromBin(uint nodeIndex)
    {
        Node &node = mNodes[nodeIndex];
    
        if (node.BinListPrev != OFFSET_ALLOCATOR_INVALID) {
            mNodes[node.BinListPrev].BinListNext = node.BinListNext;
            if (node.BinListNext != OFFSET_ALLOCATOR_INVALID) mNodes[node.BinListNext].BinListPrev = node.BinListPrev;
        } else {
            uint32 binIndex = UIntToFloatRoundDown(node.DataSize);
            uint32 topBinIndex = binIndex >> OFFSET_ALLOCATOR_TOP_BINS_INDEX_SHIFT;
            uint32 leafBinIndex = binIndex & OFFSET_ALLOCATOR_LEAF_BINS_INDEX_MASK;
            
            mBinIndices[binIndex] = node.BinListNext;
            if (node.BinListNext != OFFSET_ALLOCATOR_INVALID) mNodes[node.BinListNext].BinListPrev = OFFSET_ALLOCATOR_INVALID;

            if (mBinIndices[binIndex] == OFFSET_ALLOCATOR_INVALID)
            {
                mUsedBins[topBinIndex] &= ~(1 << leafBinIndex);
                if (mUsedBins[topBinIndex] == 0) {
                    mUsedBinsTop &= ~(1 << topBinIndex);
                }
            }
        }

        mFreeNodes[++mFreeOffset] = nodeIndex;
        mFreeStorage -= node.DataSize;
    }
}
