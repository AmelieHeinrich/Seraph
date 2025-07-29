//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 17:09:46
//

#include "KC_String.h"

#include <KernelOS/KOS_DirectAllocation.h>

#include <cstdarg>
#include <cstdio> 
#include <sstream>

#undef min

namespace KC {
    KC::String StringUtil::Format(const char* fmt, ...)
    {
        va_list args1;
        va_start(args1, fmt);

        va_list args2;
        va_copy(args2, args1);
        int required = vsnprintf(nullptr, 0, fmt, args2);
        va_end(args2);

        if (required < 0) {
            va_end(args1);
            return "";
        }

        String result;
        result.resize(required);

        vsnprintf(result.data(), result.size() + 1, fmt, args1);
        va_end(args1);

        return result;
    }

    Array<KC::String> StringUtil::ToLines(const KC::String& source)
    {
        Array<KC::String> lines;
        std::istringstream stream(source);
        KC::String line;
        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back(); // Remove '\r' from \r\n
            }
            lines.push_back(line);
        }
        return lines;
    }

    void StringUtil::TrimLeft(String& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }));
    }

    void StringUtil::TrimRight(String& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) { return !std::isspace(static_cast<unsigned char>(ch)); }).base(), s.end());
    }

    void StringUtil::Trim(String& s)
    {
        TrimLeft(s);
        TrimRight(s);
    }

    bool StringUtil::StartsWith(const String& s, const String& prefix)
    {
        return s.compare(0, prefix.size(), prefix) == 0;
    }
}
