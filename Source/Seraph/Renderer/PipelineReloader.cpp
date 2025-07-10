//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-10 22:05:51
//

#include "PipelineReloader.h"

PipelineReloader::Data PipelineReloader::sData;

void PipelineReloader::Initialize(IRHIDevice* device)
{
    sData.Device = device;
    sData.Start = SDL_NS_TO_SECONDS(SDL_GetTicksNS());
}

void PipelineReloader::Shutdown()
{
    for (auto& [_, entry] : sData.Entries) {
        switch (entry.Type) {
            case PipelineType::kGraphics: delete entry.GraphicsPipeline;
            case PipelineType::kCompute: delete entry.ComputePipeline;
            case PipelineType::kMesh: delete entry.MeshPipeline;
        }
    }
    sData.Entries.clear();
}

void PipelineReloader::SubscribeGraphics(const String& path, const RHIGraphicsPipelineDesc& desc, const Array<String>& entryPoint)
{
    PipelineEntry entry = {};
    entry.Type = PipelineType::kGraphics;
    entry.GraphicsDesc = desc;
    entry.EntryPoints = entryPoint;
    entry.ShaderFile = path;
    entry.Dependencies.push_back({
        "Data/Shaders/" + path,
        FileSystem::GetWriteTime("Data/Shaders/" + path)
    });
    Array<String> lines = FileSystem::ReadAllLines("Data/Shaders/" + path);
    for (String line : lines) {
        // Trim leading/trailing whitespace
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

        // Check for #include "..."
        if (line.compare(0, 10, "#include \"") == 0) {
            size_t start = line.find('\"') + 1;
            size_t end = line.find_last_of('\"');
            if (start != String::npos && end != String::npos && end > start) {
                String filename = line.substr(start, end - start);
                
                entry.Dependencies.push_back({
                    "Data/Shaders/" + filename,
                    FileSystem::GetWriteTime("Data/Shaders/" + filename)
                });
            }
        }
    }

    sData.Entries[path] = entry;
}

void PipelineReloader::SubscribeCompute(const String& path, const RHIComputePipelineDesc& desc, const String& entryPoint)
{
    PipelineEntry entry = {};
    entry.Type = PipelineType::kCompute;
    entry.ComputeDesc = desc;
    entry.ShaderFile = path;
    entry.EntryPoints.push_back(entryPoint);
    entry.Dependencies.push_back({
        "Data/Shaders/" + path,
        FileSystem::GetWriteTime("Data/Shaders/" + path)
    });
    Array<String> lines = FileSystem::ReadAllLines("Data/Shaders/" + path);
    for (String line : lines) {
        // Trim leading/trailing whitespace
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

        // Check for #include "..."
        if (line.compare(0, 10, "#include \"") == 0) {
            size_t start = line.find('\"') + 1;
            size_t end = line.find_last_of('\"');
            if (start != String::npos && end != String::npos && end > start) {
                String filename = line.substr(start, end - start);
                
                entry.Dependencies.push_back({
                    "Data/Shaders/" + filename,
                    FileSystem::GetWriteTime("Data/Shaders/" + filename)
                });
            }
        }
    }

    sData.Entries[path] = entry;
}

void PipelineReloader::SubscribeMesh(const String& path, const RHIMeshPipelineDesc& desc, const Array<String>& entryPoint)
{
    PipelineEntry entry = {};
    entry.Type = PipelineType::kMesh;
    entry.MeshDesc = desc;
    entry.EntryPoints = entryPoint;
    entry.ShaderFile = path;
    entry.Dependencies.push_back({
        "Data/Shaders/" + path,
        FileSystem::GetWriteTime("Data/Shaders/" + path)
    });
    Array<String> lines = FileSystem::ReadAllLines("Data/Shaders/" + path);
    for (String line : lines) {
        // Trim leading/trailing whitespace
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), line.end());

        // Check for #include "..."
        if (line.compare(0, 10, "#include \"") == 0) {
            size_t start = line.find('\"') + 1;
            size_t end = line.find_last_of('\"');
            if (start != String::npos && end != String::npos && end > start) {
                String filename = line.substr(start, end - start);
                
                entry.Dependencies.push_back({
                    "Data/Shaders/" + filename,
                    FileSystem::GetWriteTime("Data/Shaders/" + filename)
                });
            }
        }
    }

    sData.Entries[path] = entry;
}

IRHIGraphicsPipeline* PipelineReloader::GetGraphics(const String& path)
{
    return sData.Entries[path].GraphicsPipeline;
}

IRHIComputePipeline* PipelineReloader::GetCompute(const String& path)
{
    return sData.Entries[path].ComputePipeline;
}

IRHIMeshPipeline* PipelineReloader::GetMesh(const String& path)
{
    return sData.Entries[path].MeshPipeline;
}

void PipelineReloader::ReloadPipelines(bool force)
{
    if (!force) {
        float seconds = SDL_NS_TO_SECONDS(SDL_GetTicksNS());
        if (seconds - sData.Start < 0.5f)
            return;
        sData.Start = seconds;
    }

    for (auto& [_, entry] : sData.Entries) {
        for (auto& dependency : entry.Dependencies) {
            FileTime time = FileSystem::GetWriteTime(dependency.Path);
            if (time != dependency.LastWritten || force) {
                dependency.LastWritten = time;

                // Reload. PERSONAAAA
                switch (entry.Type) {
                    case PipelineType::kGraphics: {
                        CompiledShader shader = ShaderCompiler::Compile(entry.ShaderFile);
                        if (shader.Entries.empty()) break;
                        if (entry.GraphicsPipeline) delete entry.GraphicsPipeline;
                        entry.GraphicsDesc.Bytecode.clear();

                        // Compile shader
                        for (auto& [name, bytecode] : shader.Entries) {
                            entry.GraphicsDesc.Bytecode[bytecode.Stage] = bytecode;
                        }

                        entry.GraphicsPipeline = sData.Device->CreateGraphicsPipeline(entry.GraphicsDesc);
                        SERAPH_INFO("Hot reloaded %s", dependency.Path.c_str());
                        break;
                    }
                    case PipelineType::kCompute: {
                        CompiledShader shader = ShaderCompiler::Compile(entry.ShaderFile);
                        if (shader.Entries.empty()) break;

                        if (entry.ComputePipeline) delete entry.ComputePipeline;
                        entry.ComputeDesc.ComputeBytecode.Bytecode.clear();
                        entry.ComputeDesc.ComputeBytecode = shader.Entries[entry.EntryPoints[0]];

                        entry.ComputePipeline = sData.Device->CreateComputePipeline(entry.ComputeDesc);
                        SERAPH_INFO("Hot reloaded %s", dependency.Path.c_str());
                        break;
                    }
                    case PipelineType::kMesh: {
                        CompiledShader shader = ShaderCompiler::Compile(entry.ShaderFile);
                        if (shader.Entries.empty()) break;
                        if (entry.MeshPipeline) delete entry.MeshPipeline;
                        entry.MeshDesc.Bytecode.clear();
                        
                        // Compile shader
                        for (auto& [name, bytecode] : shader.Entries) {
                            entry.MeshDesc.Bytecode[bytecode.Stage] = bytecode;
                        }

                        entry.MeshPipeline = sData.Device->CreateMeshPipeline(entry.MeshDesc);
                        SERAPH_INFO("Hot reloaded %s", dependency.Path.c_str());
                        break;
                    }
                }
                break;
            }
        }
    }
}
