//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 23:12:57
//

#include "KOS_DynLib.h"
#include <Windows.h>

namespace KOS
{
    DynLib::DynLib(const char* path)
    {
        mHandle = LoadLibraryA(path);
    }

    DynLib::~DynLib()
    {
        if (mHandle) FreeModule((HMODULE)mHandle);
    }

    void* DynLib::GetFunction(const char* name)
    {
        return GetProcAddress(reinterpret_cast<HMODULE>(mHandle), name);
    }
}
