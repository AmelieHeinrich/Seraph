//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:35:32
//

#pragma once

#include "KC_Stream.h"
#include "KC_String.h"

namespace KC
{
    class StringStream : public IStream
    {
    public:
        uint64 Write(const void* data, uint64 size) override;
        void Write(const String& str);

        uint64 Read(void* data, uint64 size) override;

        bool Seek(uint64 position) override;
        uint64 Tell() const override;
        uint64 Size() const override;

        String Buffer() { return mBuffer; }
    private:
        String mBuffer;
        uint mPosition = 0;
    };
}
