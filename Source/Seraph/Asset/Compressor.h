//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 19:47:44
//

#pragma once

#include <Core/Context.h>
#include <nvtt/nvtt.h>

class Compressor
{
public:
    void RecurseFolder(const String& path);
    void CompressTexture(const String& path);

    static String ToCachedPath(const String& path);
private:
    nvtt::Context mContext;
};
