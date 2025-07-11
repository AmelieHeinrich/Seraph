//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 19:47:44
//

#pragma once

#include <Core/Context.h>

class Compressor
{
public:
    void RecurseFolder(const std::string& path);
    void CompressTexture(const std::string& path);

    static std::string ToCachedPath(const std::string& path);
};
