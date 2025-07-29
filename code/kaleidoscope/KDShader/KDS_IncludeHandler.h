//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:29:32
//

#pragma once

#include "KDS_Parser.h"

namespace KDS
{
    class IncludeHandler
    {
    public:
        IncludeHandler();
        ~IncludeHandler();

        void AddIncludePath(const KC::String& path);
        KC::String ReplaceIncludes(const ParseResult& result, const KC::String& source, KC::Array<KC::String>& outFoundPaths);
    private:
        KC::Array<KC::String> mIncludePaths;
    };
}