//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 13:33:31
//

#pragma once

#include "Types.h"

#include <JSON/json.hpp>

class FileSystem
{
public:
    static void Initialize();
    static void Shutdown();

    static nlohmann::json ReadJSON(const String& path);
    static void WriteJSON(nlohmann::json json, const String& path);

    static bool Exists(const String& path);
private:
    static struct Data {
        String WorkingDirectory;
    } sData;
};
