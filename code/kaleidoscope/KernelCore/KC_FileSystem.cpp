//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:40:08
//

#include "KC_FileSystem.h"
#include "KC_Context.h"

#include <filesystem>

namespace KC
{
    bool FileSystem::Exists(const String& path)
    {
        KOS::File file;
        file.Open(path.c_str(), KOS::FileOpenMode::kRead);
        if (!file.IsOpen())
            return false;
        file.Close();
        return true;
    }

    String FileSystem::GetExtension(const String& path)
    {
        size_t pos = path.find_last_of('.');
        if (pos == String::npos)
            return "";
        return path.substr(pos + 1);
    }

    String FileSystem::ReadWholeFile(const KC::String& path)
    {
        KOS::File file;
        file.Open(path.c_str(), KOS::FileOpenMode::kRead);
        if (!file.IsOpen()) {
            KD_ERROR("Failed to read file! %s", path.c_str());
            return "";
        }

        KC::String result;
        result.resize(file.Size());
        file.Read(result.data(), file.Size());
        file.Close();

        return result;
    }

    void FileSystem::CreateDirectory(const String& path)
    {
        std::filesystem::create_directory(path);
    }

    FileTime FileSystem::GetWriteTime(const KC::String& path)
    {
        return std::filesystem::last_write_time(path);
    }
}
