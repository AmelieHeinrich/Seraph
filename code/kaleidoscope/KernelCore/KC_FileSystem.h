//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:39:34
//

#pragma once

#include <KernelOS/KOS_File.h>
#include <filesystem>

#include "KC_String.h"

namespace KC
{
    using FileTime = std::filesystem::file_time_type;

    class FileSystem
    {
    public:
        static bool Exists(const String& path);
        static String GetExtension(const String& path);
        
        static String ReadWholeFile(const String& path);

        static void CreateDirectory(const String& path);

        static FileTime GetWriteTime(const KC::String& path);
    };
}
