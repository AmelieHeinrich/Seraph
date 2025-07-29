//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 19:56:12
//

#pragma once

#include <KernelOS/KOS_DynLib.h>

namespace KDS
{
    class Manager
    {
    public:
        static void Initialize();
        static void Shutdown();

        static KOS::DynLib* GetDynLib();
    private:
        static KOS::DynLib* sDynLib;
    };
}
