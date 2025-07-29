//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 19:57:44
//

#include "KDS_Manager.h"

#include <KernelCore/KC_Context.h>

namespace KDS
{
    KOS::DynLib* Manager::sDynLib;

    void Manager::Initialize()
    {
        sDynLib = KC_NEW(KOS::DynLib, DXC_DLL);
    }

    void Manager::Shutdown()
    {
        KC_DELETE(sDynLib);
    }

    KOS::DynLib* Manager::GetDynLib()
    {
        return sDynLib;
    }
}
