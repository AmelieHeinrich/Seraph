//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:17:05
//

#pragma once

#include "KGPU_Buffer.h"

namespace KGPU
{
    struct BLASDesc
    {
        IBuffer* VertexBuffer;
        IBuffer* IndexBuffer;
        uint VertexCount;
        uint IndexCount;
        bool Static;
    
        BLASDesc() = default;
        BLASDesc(IBuffer* v, IBuffer* b)
            : VertexBuffer(v), IndexBuffer(b), Static(true)
        {
            VertexCount = v->GetDesc().Size / v->GetDesc().Size;
            IndexCount = b->GetDesc().Size / b->GetDesc().Stride;
        }
    };
    
    class IBLAS
    {
    public:
        virtual ~IBLAS() = default;
    
        BLASDesc GetDesc() const { return mDesc; }
        
        virtual uint64 GetAddress() = 0;
    
        IBuffer* GetMemory() { return mMemory; }
        IBuffer* GetScratch() { return mScratch; }
    protected:
        BLASDesc mDesc;
    
        IBuffer* mMemory;
        IBuffer* mScratch;
    };
}
