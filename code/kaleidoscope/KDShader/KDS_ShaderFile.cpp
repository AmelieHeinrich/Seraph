//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-16 22:40:36
//

#include "KDS_ShaderFile.h"
#include "KDS_IncludeHandler.h"

#include <KernelCore/KC_FileSystem.h>

namespace KDS
{
    ShaderFile::ShaderFile(ICompiler* compiler, IReflectionEngine* engine, const KC::String& path)
    {
        KC::String source = KC::FileSystem::ReadWholeFile(path);
        
        Parser parser;
        ParseResult parseResult = parser.ParseShaderSource(source);
        KC::Array<KC::String> found;
        
        IncludeHandler handler;
        handler.AddIncludePath("data/");
        handler.AddIncludePath("data/kd/shaders");
        handler.AddIncludePath("data/rf/shaders");
        source = handler.ReplaceIncludes(parseResult, source, found);
    
        for (auto& entryPoint : parseResult.EntryPoints) {
            KGPU::ShaderModule module = compiler->Compile(source, entryPoint.Name, entryPoint.Stage);
            mModules[entryPoint.Stage] = module;
            if (mReflection.PushConstantSize == 0)
                mReflection = engine->Reflect(module);
        }

        KD_INFO("Compiled shader file '%s'", path.c_str());
    }

    ShaderFile::~ShaderFile()
    {
        mModules.clear();
    }

    KGPU::ShaderModule ShaderFile::Get(KGPU::ShaderStage stage)
    {
        return mModules[stage];
    }
}
