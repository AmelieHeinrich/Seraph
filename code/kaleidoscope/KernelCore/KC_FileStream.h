//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:48:32
//

#pragma once

#include "KC_Stream.h"
#include "KC_String.h"

#include <KernelOS/KOS_File.h>

namespace KC
{
    class FileStream : public IStream
    {
    public:
        FileStream();
        FileStream(const String& path, KOS::FileOpenMode mode);
        ~FileStream();

        void Open(const String& path, KOS::FileOpenMode mode);
        void Close();
        bool IsOpen() const;

        uint64 Write(const void* data, uint64 size) override;
        uint64 Write(const String& str);

        uint64 Read(void* data, uint64 size) override;

        bool Seek(uint64 position) override;
        uint64 Tell() const override;
        uint64 Size() const override;

    private:
        KOS::File mFile;
    };
}
