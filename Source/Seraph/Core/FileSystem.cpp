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

bool FileSystem::Exists(const String& path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == -1)
        return false;
    return true;   
}
