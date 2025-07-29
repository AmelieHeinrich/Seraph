//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 16:27:23
//

#include "KOS_Assert.h"

#include <Windows.h>

namespace KOS {
    void Assert::AssertBox(const char* msg)
    {
        MessageBoxA(nullptr, msg, "KALEIDOSCOPE ASSERT FAILED", MB_OK | MB_ICONERROR);
        __debugbreak();
    }
}
