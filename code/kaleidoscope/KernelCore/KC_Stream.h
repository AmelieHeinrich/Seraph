//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:32:27
//

#pragma once

#include "KC_Common.h"

namespace KC
{
    class IStream
    {
    public:
        virtual ~IStream() {}

        virtual uint64 Write(const void* data, uint64 size) = 0;
        virtual uint64 Read(void* data, uint64 size) = 0;
        virtual bool Seek(uint64 position) = 0;
        virtual uint64 Tell() const = 0;
        virtual uint64 Size() const = 0;
    };
}
