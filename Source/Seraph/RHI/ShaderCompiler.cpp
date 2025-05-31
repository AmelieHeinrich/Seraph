//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-29 21:05:59
//

#include "ShaderCompiler.h"

ShaderCompiler::Data ShaderCompiler::sData;

void ShaderCompiler::Initialize(RHIBackend backend)
{
    sData.Backend = backend;

    SlangGlobalSessionDesc desc = {};
    
    SlangResult result = slang::createGlobalSession(&desc, &sData.GlobalSession);
    ASSERT_EQ(SLANG_SUCCEEDED(result), "Failed to initialize Slang global session!");

    SERAPH_INFO("Initialized Slang global session!");
}

void ShaderCompiler::Shutdown()
{
    sData.GlobalSession->Release();
}

CompiledShader ShaderCompiler::Compile(const String& path, Array<String> entryPoints)
{
    CompiledShader result = {};

    const char* searchPaths[] = { "Shaders" };

    slang::TargetDesc targetDesc = {};
    targetDesc.format = sData.Backend == RHIBackend::kVulkan ? SLANG_SPIRV : SLANG_DXIL;
    targetDesc.profile = sData.GlobalSession->findProfile("sm_6_6");

    slang::CompilerOptionEntry matrixLayoutEntry;
    matrixLayoutEntry.name = slang::CompilerOptionName::MatrixLayoutColumn;

    slang::CompilerOptionEntry debugEntry;
    debugEntry.name = slang::CompilerOptionName::DebugInformation;
    debugEntry.value.intValue0 = SlangDebugInfoLevel::SLANG_DEBUG_INFO_LEVEL_MAXIMAL;

    slang::PreprocessorMacroDesc platformDesc = {};
    platformDesc.value = "1";
    platformDesc.name = sData.Backend == RHIBackend::kVulkan ? "VULKAN" : "D3D12";

    slang::CompilerOptionEntry entries[] = { matrixLayoutEntry, debugEntry };

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.searchPaths = searchPaths;
    sessionDesc.searchPathCount = 1;
    sessionDesc.compilerOptionEntries = entries;
    sessionDesc.compilerOptionEntryCount = 2;
    sessionDesc.preprocessorMacroCount = 1;
    sessionDesc.preprocessorMacros = &platformDesc;

    Slang::ComPtr<slang::ISession> session = nullptr;
    sData.GlobalSession->createSession(sessionDesc, session.writeRef());

    Slang::ComPtr<slang::IBlob> diagnostics = nullptr;
    Slang::ComPtr<slang::IModule> module(session->loadModule(path.c_str(), diagnostics.writeRef()));
    if (diagnostics) {
        SERAPH_ERROR("Failed to compile shader %s : %s", path.c_str(), diagnostics->getBufferPointer());
        return {};
    }

    Array<Slang::ComPtr<slang::IEntryPoint>> slangEntryPoints;
    for (String entryPoint : entryPoints) {
        Slang::ComPtr<slang::IEntryPoint> slangEntryPoint;
        module->findEntryPointByName(entryPoint.c_str(), slangEntryPoint.writeRef());
        slangEntryPoints.push_back(slangEntryPoint);
    }

    Array<slang::IComponentType*> components;
    components.push_back(module);
    for (Slang::ComPtr<slang::IEntryPoint> entryPoint : slangEntryPoints) {
        components.push_back(entryPoint);
    }

    Slang::ComPtr<slang::IComponentType> program = nullptr;
    session->createCompositeComponentType(components.data(), components.size(), program.writeRef());

    Slang::ComPtr<slang::IComponentType> linkedProgram = nullptr;
    Slang::ComPtr<ISlangBlob> diagnosticBlob = nullptr;
    program->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());

    for (int i = 0; i < entryPoints.size(); i++) {
        Slang::ComPtr<slang::IBlob> kernelBlob;
        linkedProgram->getEntryPointCode(i, 0, kernelBlob.writeRef());
        
        ShaderModule shaderModule = {};
        shaderModule.Entry = entryPoints[i];
        shaderModule.Bytecode.resize(kernelBlob->getBufferSize());
        memcpy(shaderModule.Bytecode.data(), kernelBlob->getBufferPointer(), shaderModule.Bytecode.size());
        result.Entries[entryPoints[i]] = shaderModule;
    }

    SERAPH_INFO("Compiled shader : %s", path.c_str());

    return result;
}
