//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:34:30
//

#pragma once

#include "KC_Log.h"

namespace KC
{
    class Assert
    {
    public:
        static void Eq(bool condition, const String& file, const String& function, int line, const char* message);
    };
}

#define KD_ASSERT_EQ(cond, msg) KC::Assert::Eq(cond, __FILE__, __FUNCTION__, __LINE__, msg);
