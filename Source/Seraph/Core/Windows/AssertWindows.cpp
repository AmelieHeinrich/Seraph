//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:35:03
//

#include "Core/Assert.h"
#include "Core/Context.h"

#include <Windows.h>

void Assert::Eq(bool condition, const std::string& file, const std::string& function, int line, const std::string& message)
{
    if (!condition) {
        SERAPH_FATAL(file.c_str(), line, "ASSERTION FAILED (%s:%s - line %d): %s", file.c_str(), function.c_str(), line, message.c_str());
        MessageBoxA(nullptr, "Assertion Failed!", "NOISE", MB_OK | MB_ICONERROR);
        __debugbreak();
    }
}

