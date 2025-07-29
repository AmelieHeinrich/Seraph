//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:35:17
//

#include "KC_Assert.h"
#include "KC_Context.h"

#include <KernelOS/KOS_Assert.h>

namespace KC {
    // TODO(Amélie Heinrich) : Get call stack?

    void Assert::Eq(bool condition, const String& file, const String& function, int line, const char* data)
    {
        if (!condition) {
            KC::String message = KC::StringUtil::Format("ASSERTION FAILED (%s:%s - line %d): %s", file.c_str(), function.c_str(), line, data);
            KD_FATAL("%s", message.c_str());
            KOS::Assert::AssertBox(message.c_str());
        }
    }
}
