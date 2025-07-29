//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 23:11:48
//

#pragma once

#include "KOS_Common.h"

namespace KOS
{
    class DynLib
    {
    public:
        DynLib(const char* path);
        ~DynLib();

        void* GetFunction(const char* name);
        bool Valid() const { return mHandle != nullptr; }
    private:
        void* mHandle = nullptr;
    };

#ifdef KD_WINDOWS
    #define DXC_DLL "dxcompiler.dll"
    #define DISCORD_DLL "discord_game_sdk.dll"
#endif
}
