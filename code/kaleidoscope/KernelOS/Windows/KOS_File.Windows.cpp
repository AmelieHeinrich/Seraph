//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:44:08
//

#include "KOS_File.h"

#include <Windows.h>

namespace KOS {
    File::File()
    {
        mHandle = INVALID_HANDLE_VALUE;
    }

    File::File(const char* path, FileOpenMode mode)
    {
        Open(path, mode);
    }

    File::~File()
    {
        Close();
    }

    bool File::Open(const char* path, FileOpenMode mode)
    {
        DWORD access = 0, creation = 0;

        if (Any(mode & FileOpenMode::kRead)) access |= GENERIC_READ;
        if (Any(mode & FileOpenMode::kWrite)) access |= GENERIC_WRITE;

        if (Any(mode & FileOpenMode::kOverwrite)) creation = CREATE_ALWAYS;
        else if (Any(mode & FileOpenMode::kWrite)) creation = OPEN_ALWAYS;
        else creation = OPEN_EXISTING;

        mHandle = CreateFileA(path, access, FILE_SHARE_READ, nullptr, creation, FILE_ATTRIBUTE_NORMAL, nullptr);
        return mHandle != INVALID_HANDLE_VALUE;
    }

    void File::Close()
    {
        if (mHandle != INVALID_HANDLE_VALUE) {
            CloseHandle(mHandle);
            mHandle = INVALID_HANDLE_VALUE;
        }
    }

    uint64 File::Read(void* buffer, uint64 size)
    {
        DWORD bytesRead = 0;
        ReadFile(mHandle, buffer, static_cast<DWORD>(size), &bytesRead, nullptr);
        return static_cast<uint64>(bytesRead);
    }

    uint64 File::Write(const void* data, uint64 size)
    {
        DWORD bytesWritten = 0;
        WriteFile(mHandle, data, static_cast<DWORD>(size), &bytesWritten, nullptr);
        return static_cast<uint64>(bytesWritten);
    }

    bool File::Seek(uint64 pos)
    {
        return SetFilePointer(mHandle, static_cast<LONG>(pos), nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER;
    }

    uint64 File::Tell() const
    {
        return static_cast<uint64>(SetFilePointer(mHandle, 0, nullptr, FILE_CURRENT));
    }

    uint64 File::Size() const
    {
        LARGE_INTEGER size;
        GetFileSizeEx(mHandle, &size);
        return static_cast<uint64>(size.QuadPart);
    }

    bool File::IsOpen() const
    {
        return mHandle != INVALID_HANDLE_VALUE;
    }
}
