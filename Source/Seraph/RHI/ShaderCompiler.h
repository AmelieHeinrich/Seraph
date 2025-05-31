//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:01:03
//

#pragma once

#include <Core/Context.h>

#include <slang.h>
#include <slang-com-ptr.h>
#include <slang-com-helper.h>

#include "Backend.h"

enum class ShaderStage
{
    kVertex,
    kFragment,
    kCompute,
    kGeometry,
    kTessellationEval,
    kTessellationControl,
    kMesh,
    kAmplification,
    kRayGen,
    kClosestHit,
    kAnyHit,
    kMiss
};

struct ShaderModule
{
    ShaderStage Stage;
    String Entry;
    Array<uint8> Bytecode;
};

struct CompiledShader
{
    UnorderedMap<String, ShaderModule> Entries;
};

class ShaderCompiler
{
public:
    static void Initialize(RHIBackend backend);
    static void Shutdown();

    static CompiledShader Compile(const String& path, Array<String> entryPoints);

private:
    static struct Data {
        RHIBackend Backend;

        slang::IGlobalSession* GlobalSession;
    } sData;
};
