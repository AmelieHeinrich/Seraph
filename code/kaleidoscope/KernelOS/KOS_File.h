//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:41:37
//

#pragma once

#include "KOS_Common.h"

namespace KOS
{
    enum class FileOpenMode {
        kRead = BIT(1),
        kWrite = BIT(2),
        kOverwrite = BIT(3)
    };
    ENUM_CLASS_FLAGS(FileOpenMode);

    class File
    {
    public:
        File();
        File(const char* path, FileOpenMode mode);
        ~File();

        bool Open(const char* path, FileOpenMode mode);
        void Close();

        uint64 Read(void* buffer, uint64 size);
        uint64 Write(const void* data, uint64 size);

        bool Seek(uint64 pos);
        uint64 Tell() const;
        uint64 Size() const;

        bool IsOpen() const;
    private:
#if KD_WINDOWS
        void* mHandle;
#endif
    };
}
