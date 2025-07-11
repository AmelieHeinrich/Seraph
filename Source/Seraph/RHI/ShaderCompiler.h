//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:01:03
//

#pragma once

#include <Core/Context.h>

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
    std::string Entry;
    std::vector<uint8> Bytecode;
};

struct CompiledShader
{
    UnorderedMap<std::string, ShaderModule> Entries;
};

class ShaderCompiler
{
public:
    static void Initialize(RHIBackend backend);
    static void Shutdown();

    static CompiledShader Compile(const std::string& path);

private:
    static struct Data {
        RHIBackend Backend;
    } sData;
};
