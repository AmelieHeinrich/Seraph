//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:38:18
//

#include "KC_StringStream.h"

#undef min

namespace KC {
    uint64 StringStream::Write(const void* data, uint64 size)
    {
        if (mPosition + size > mBuffer.size()) {
            mBuffer.resize(mPosition + size);
        }
        memcpy(&mBuffer[mPosition], data, size);
        mPosition += size;
        return size;
    }

    void StringStream::Write(const KC::String& str)
    {
        Write(str.data(), str.size());
    }

    uint64 StringStream::Read(void* data, uint64 size)
    {
        uint64 available = mBuffer.size() - mPosition;
        uint64 toRead = std::min(size, available);
        memcpy(data, mBuffer.c_str() + mPosition, toRead);
        mPosition += toRead;
        return toRead;
    }

    bool StringStream::Seek(uint64 position)
    {
        if (position <= mBuffer.size()) {
            mPosition = position;
            return true;
        }
        return false;
    }

    uint64 StringStream::Tell() const {
        return mPosition;
    }

    uint64 StringStream::Size() const {
        return mBuffer.size();
    }
}
