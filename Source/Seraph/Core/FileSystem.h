//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 13:33:31
//

#pragma once

#include "Types.h"

class FileSystem
{
public:
    static void Initialize();
    static void Shutdown();

    static bool Exists(const String& path);
private:
    static struct Data {
        String WorkingDirectory;
    } sData;
};
