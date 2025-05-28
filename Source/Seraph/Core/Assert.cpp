//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:35:03
//

#include "Assert.h"
#include "Context.h"

#include <Windows.h>

void Assert::Eq(bool condition, const String& file, const String& function, int line, const String& message)
{
    if (!condition) {
        SERAPH_FATAL(file.c_str(), line, "ASSERTION FAILED (%s:%s - line %d): %s", file.c_str(), function.c_str(), line, message.c_str());
        MessageBoxA(nullptr, "Assertion Failed!", "NOISE", MB_OK | MB_ICONERROR);
        __debugbreak();
    }
}
