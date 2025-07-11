//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 13:33:31
//

#pragma once

#include "Types.h"

#include <JSON/json.hpp>
#include <filesystem>

using FileTime = std::filesystem::file_time_type;

class FileSystem
{
public:
    static void Initialize();
    static void Shutdown();

    static nlohmann::json ReadJSON(const std::string& path);
    static void WriteJSON(nlohmann::json json, const std::string& path);

    static std::vector<std::string> ReadAllLines(const std::string& path);
    static std::string ReadFile(const std::string& path);

    static bool Exists(const std::string& path);
    static uint GetFileSize(const std::string& path);
    
    static FileTime GetWriteTime(const std::string& path);
private:
    static struct Data {
        std::string WorkingDirectory;
    } sData;
};
