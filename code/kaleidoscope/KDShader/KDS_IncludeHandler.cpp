//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 20:33:21
//

#include "KDS_IncludeHandler.h"

#include <KernelCore/KC_FileSystem.h>
#include <KernelCore/KC_StringStream.h>

namespace KDS
{
    IncludeHandler::IncludeHandler()
    {
        mIncludePaths.clear();
        mIncludePaths.push_back(".");
    }

    IncludeHandler::~IncludeHandler()
    {
        mIncludePaths.clear();
    }

    void IncludeHandler::AddIncludePath(const KC::String& path)
    {
        mIncludePaths.push_back(path);
    }

    KC::String IncludeHandler::ReplaceIncludes(const ParseResult& result, const KC::String& source, KC::Array<KC::String>& outFoundPaths)
    {
        KC::Array<KC::String> lines = KC::StringUtil::ToLines(source);

        for (const IncludeRecord& include : result.Includes)
        {
            KC::String includeContent = "";
            bool found = false;

            // Search for the include file in the include paths
            for (const KC::String& path : mIncludePaths) {
                KC::String fullPath = KC::StringUtil::Format("%s/%s", path.c_str(), include.Path.c_str());

                if (KC::FileSystem::Exists(fullPath)) {
                    includeContent = KC::FileSystem::ReadWholeFile(fullPath);
                    found = true;
                    outFoundPaths.push_back(fullPath);
                    break;
                }
            }

            if (!found) {
                KD_WARN("IncludeHandler: Could not find include file '%s'", include.Path.c_str());
                continue;
            }

            // Insert the file content at the correct line (line is 1-based)
            lines[include.Line] = includeContent;
        }

        // Rebuild the final string
        KC::StringStream stream;
        for (const KC::String& line : lines) {
            stream.Write(line);
            stream.Write("\n");
        }

        return stream.Buffer();
    }
}
