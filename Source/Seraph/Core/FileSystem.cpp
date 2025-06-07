//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-03 13:36:24
//

#include "FileSystem.h"
#include "Context.h"

#include <filesystem>
#include <sys/stat.h>
#include <fstream>

FileSystem::Data FileSystem::sData;

void FileSystem::Initialize()
{
    sData.WorkingDirectory = std::filesystem::current_path().string();
    for (auto& character : sData.WorkingDirectory) {
        if (character == '\\') character = '/';
    }

    SERAPH_INFO("Initialized filesystem. CWD: %s", sData.WorkingDirectory.c_str());
}

void FileSystem::Shutdown()
{

}

nlohmann::json FileSystem::ReadJSON(const String& path)
{
    std::ifstream stream(path);
    if (!stream.is_open()) {
        SERAPH_ERROR("Failed to open JSON file {0}", path);
        return {};
    }
    nlohmann::json root = nlohmann::json::parse(stream);
    stream.close();
    return root;
}

void FileSystem::WriteJSON(nlohmann::json json, const String& path)
{
    std::ofstream stream(path);
    if (!stream.is_open()) {
        SERAPH_ERROR("Failed to open JSON file {0} for writing!", path);
    }
    stream << json.dump(4) << std::endl;
    stream.close();
}

bool FileSystem::Exists(const String& path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == -1)
        return false;
    return true;   
}

uint FileSystem::GetFileSize(const String& path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == -1)
        return 0;
    return s.st_size;  
}
