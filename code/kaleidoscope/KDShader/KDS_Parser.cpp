//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 13:20:00
//

#include "KDS_Parser.h"

namespace KDS
{
    ParseResult Parser::ParseShaderSource(const KC::String& source)
    {
        ParseResult result = {};
        KC::Array<KC::String> lines = KC::StringUtil::ToLines(source);

        for (size_t i = 0; i < lines.size(); i++) {
            KC::String line = lines[i];
            KC::StringUtil::Trim(line);

            if (line.empty()) {
                continue;
            }

            // --- Handle #include ---
            const std::string includeMacro = "#include \"";

            KC::String includePath;
            if (KC::StringUtil::StartsWith(line, includeMacro)) {
                KC::String includeName = line.substr(includeMacro.size());
                KC::StringUtil::Trim(includeName);
                        
                int last = includeName.find_last_of('\"');
                if (last != KC::String::npos) {
                    includeName = includeName.substr(0, last);
                    result.Includes.push_back(KDS::IncludeRecord{ includeName, static_cast<int>(i) });
                }
            }

            // --- Handle #pragma ---
            const std::string pragmaVertex = "#pragma vertex";
            const std::string pragmaPixel = "#pragma pixel";
            const std::string pragmaCompute = "#pragma compute";

            if (KC::StringUtil::StartsWith(line, pragmaVertex)) {
                KC::String funcName = line.substr(pragmaVertex.size());
                KC::StringUtil::Trim(funcName);
                result.EntryPoints.push_back(EntryPointRecord{
                    KGPU::ShaderStage::kVertex,
                    funcName
                 });
            }
            else if (KC::StringUtil::StartsWith(line, pragmaPixel)) {
                KC::String funcName = line.substr(pragmaPixel.size());
                KC::StringUtil::Trim(funcName);
                result.EntryPoints.push_back(EntryPointRecord{
                    KGPU::ShaderStage::kPixel,
                    funcName
                });
            }
            else if (KC::StringUtil::StartsWith(line, pragmaCompute)) {
                KC::String funcName = line.substr(pragmaCompute.size());
                KC::StringUtil::Trim(funcName);
                result.EntryPoints.push_back(EntryPointRecord{
                    KGPU::ShaderStage::kCompute,
                    funcName
                 });
            }
        }

        return result;
    }
}
