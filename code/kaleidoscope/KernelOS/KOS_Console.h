//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 16:18:11
//

#pragma once

#include "KOS_Common.h"

namespace KOS
{
    enum class ConsoleColor
    {
        kDefault,
        kRed,
        kGreen,
        kBlue,
        kYellow,
        kWhite
    };

    class Console
    {
    public:
        static void SetColor(ConsoleColor color);
        static void Write(const char* data, uint64 size);
    };
}
