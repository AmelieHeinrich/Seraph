//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 06:59:46
//

#pragma once

#include "Types.h"

#include <cstdarg>

// Credit to https://github.com/simco50/D3D12_Research/blob/master/Source/Core/CString.h

class UTF
{
public:
    static std::string WideToAscii(const wchar_t* text);
    static std::string WideToAscii(const std::wstring& text);
    static std::wstring AsciiToWide(const char* text);
    static std::wstring AsciiToWide(const std::string& text);
};
