//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:50:19
//

#include "KC_FileStream.h"

namespace KC {
    FileStream::FileStream()
    {
    }

    FileStream::FileStream(const String& path, KOS::FileOpenMode mode)
    {
        mFile.Open(path.c_str(), mode);
    }

    FileStream::~FileStream()
    {
        Close();
    }

    void FileStream::Open(const String& path, KOS::FileOpenMode mode)
    {
        mFile.Open(path.c_str(), mode);
    }

    void FileStream::Close()
    {
        mFile.Close();
    }

    bool FileStream::IsOpen() const
    {
        return mFile.IsOpen();
    }

    uint64 FileStream::Write(const void* data, uint64 size)
    {
        return mFile.Write(data, size);
    }

    uint64 FileStream::Write(const String& str)
    {
        return mFile.Write(str.c_str(), str.size());
    }

    uint64 FileStream::Read(void* data, uint64 size)
    {
        return  mFile.Read(data, size);
    }

    bool FileStream::Seek(uint64 position)
    {
        return mFile.Seek(position);
    }

    uint64 FileStream::Tell() const
    {
        return mFile.Tell();
    }

    uint64 FileStream::Size() const
    {
        return mFile.Size();
    }
}
