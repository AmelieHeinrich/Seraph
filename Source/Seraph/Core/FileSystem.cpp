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

FileTime FileSystem::GetWriteTime(const String& path)
{
    return std::filesystem::last_write_time(path);
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

Array<String> FileSystem::ReadAllLines(const String& path)
{
     std::ifstream stream(path);
    std::vector<std::string> lines;
    std::string line;
    
    if (!stream.is_open()) {
        SERAPH_ERROR("Failed to open file! %s", path.c_str());
        return {};
    }
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    stream.close();
    return lines;
}

String FileSystem::ReadFile(const String& path)
{
    std::ifstream stream(path);
    if (!stream.is_open()) {
        SERAPH_ERROR("failed to open file! %s", path.c_str());
        return "";
    }

    std::stringstream ss;
    ss << stream.rdbuf();

    stream.close();
    return ss.str();
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
