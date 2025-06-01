//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 19:59:08
//

#pragma once

#include <Core/Context.h>

#include "Buffer.h"

struct RHIBLASDesc
{
    IRHIBuffer* VertexBuffer;
    IRHIBuffer* IndexBuffer;
    uint VertexCount;
    uint IndexCount;
    bool Static;

    RHIBLASDesc() = default;
    RHIBLASDesc(IRHIBuffer* v, IRHIBuffer* b)
        : VertexBuffer(v), IndexBuffer(b), Static(true)
    {
        VertexCount = v->GetDesc().Size / v->GetDesc().Size;
        IndexCount = b->GetDesc().Size / b->GetDesc().Stride;
    }
};

class IRHIBLAS
{
public:
    ~IRHIBLAS() = default;

    RHIBLASDesc GetDesc() const { return mDesc; }
    
    virtual uint64 GetAddress() = 0;

    IRHIBuffer* GetMemory() { return mMemory; }
    IRHIBuffer* GetScratch() { return mScratch; }
protected:
    RHIBLASDesc mDesc;

    IRHIBuffer* mMemory;
    IRHIBuffer* mScratch;
};
