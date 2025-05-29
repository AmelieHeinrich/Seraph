//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:01:03
//

#pragma once

#include <Core/Context.h>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include <slang/slang-com-helper.h>

#include "Device.h"

struct CompiledShader
{
    UnorderedMap<String, Array<uint8>> Entries;
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
