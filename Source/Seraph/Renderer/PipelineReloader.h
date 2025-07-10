//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-10 21:54:14
//

#pragma once

#include "RHI/Device.h"
#include "Core/FileSystem.h"

class PipelineReloader
{
public:
    static void Initialize(IRHIDevice* device);
    static void Shutdown();

    static void SubscribeGraphics(const String& path, const RHIGraphicsPipelineDesc& desc, const Array<String>& entryPoint);
    static void SubscribeCompute(const String& path, const RHIComputePipelineDesc& desc, const String& entryPoint);
    static void SubscribeMesh(const String& path, const RHIMeshPipelineDesc& desc, const Array<String>& entryPoint);

    static IRHIGraphicsPipeline* GetGraphics(const String& path);
    static IRHIComputePipeline* GetCompute(const String& path);
    static IRHIMeshPipeline* GetMesh(const String& path);

    // Persona 3 Reload reference?,!?:
    // Call this outside of command buffer recording/submitting!
    static void ReloadPipelines(bool force = false);
private:
    enum class PipelineType
    {
        kGraphics,
        kCompute,
        kMesh
    };

    struct FileWatch
    {
        String Path;
        FileTime LastWritten;
    };

    struct PipelineEntry
    {
        PipelineType Type;
        String ShaderFile;
        Array<FileWatch> Dependencies;
        Array<String> EntryPoints;

        IRHIComputePipeline* ComputePipeline;
        RHIComputePipelineDesc ComputeDesc;

        IRHIGraphicsPipeline* GraphicsPipeline;
        RHIGraphicsPipelineDesc GraphicsDesc;

        IRHIMeshPipeline* MeshPipeline;
        RHIMeshPipelineDesc MeshDesc;
    };

    static struct Data {
        IRHIDevice* Device;
        
        UnorderedMap<String, PipelineEntry> Entries;
        float Start = 0.0f;
    } sData;
};

/*
    Huh? So we... *reload* the pipelines? Sounds familiar...


    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣤⣤⣴⣦⡭⣑⠲⣦⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣶⣿⣿⣿⣿⣿⣿⡿⠿⠦⠝⠻⣯⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣤⣌⣉⡙⢿⣷⣦⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣾⣿⣿⣿⣷⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⢀⣴⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠿⣿⣿⠿⠿⠟⠛⡛⠁⢿⣿⣿⣿⣿⣿⣿⣿⣿⣳⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⢀⣴⢟⣥⡿⣻⣿⣿⠏⠀⠉⠛⣻⣿⣿⠟⠁⢀⣾⠟⠁⠀⠀⡀⣰⠃⠀⢀⠻⠿⠿⠿⡿⡛⢿⡻⡱⡹⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⣰⢿⣋⡾⢋⣾⣿⡿⢁⣀⣀⠀⣰⣿⡟⣁⣤⣶⢟⣵⣶⣶⣶⣾⣡⣏⠀⡠⣸⣦⣀⣀⣠⢰⡇⠀⢣⠘⢷⠘⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠐⢇⣴⣿⢠⣿⣿⣿⣷⣿⣿⣿⢰⣿⣯⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢠⣧⣿⣿⣿⣿⣿⣼⣿⡀⢀⣇⠸⡄⠸⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⢠⣾⣿⣷⣿⣿⣿⣿⣿⣿⣿⣧⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣾⣿⣿⣿⣿⣿⣿⡏⣿⡇⣼⣿⡄⢿⡀⢹⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⣿⣿⣿⣿⣿⣿⣿⣿⢿⣿⠃⣿⣿⣿⣿⣇⣸⣧⡀⠹⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠵⠒⠂⢿⣿⣿⣿⣿⣿⣿⣟⣿⣏⢀⣿⣿⣿⣿⣿⣿⣿⡇⢣⡹⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣟⣡⣤⣴⣶⠀⢸⣿⣿⣿⣿⡿⢫⣾⡟⡎⠘⠻⣿⣿⣿⣿⣿⣿⡇⢸⣇⡷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠛⣿⣿⢿⢯⡉⢹⣼⣿⣿⣿⣯⣴⣿⣿⣸⠃⡘⠻⢹⣿⣿⣿⣿⠿⠁⣼⣿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⢹⣽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠘⠹⢟⣧⣿⠖⣳⣿⣿⣿⣿⣿⣿⣿⢮⣿⡇⢮⠀⢸⣿⣿⣿⡿⠀⣠⠏⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣿⣿⠻⣄⠀⠀⠊⠙⠀⢔⣿⣿⣿⣿⣿⣿⣿⣯⣿⣿⣆⢜⣴⣿⣿⣿⣿⣣⡄⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⢿⣿⣷⡀⠈⠀⠀⠀⢀⠴⠛⣽⣿⣿⣿⣿⣿⡟⣿⣿⠟⢁⣴⣿⣿⣿⣿⣷⣿⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⢠⠾⠛⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⡏⠙⠢⠀⠀⠀⠀⠀⠀⠀⣿⡏⢿⣿⣿⠟⠴⢃⣡⡴⢿⣿⣿⣿⣿⣿⡿⠏⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠘⢿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡀⠀⣇⠀⡀⠀⠀⠀⠀⠀⠀⠀⢹⡁⢸⡟⠁⠀⠀⠘⠋⠀⠘⣿⣿⣿⣿⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⡄⠈⠀⠙⠓⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠇⠀⠀⠀⠀⠀⠀⢀⢿⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠘⣿⡿⣿⣿⣿⣿⣷⡀⠀⠀⠀⠿⠟⠛⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡢⠋⢸⣿⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠈⠉⠈⠻⣿⠹⣿⣿⣆⠀⠀⠀⠶⠆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠾⡧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣿⣷⣦⡀⠀⠀⠀⠀⠀⠀⠀⢀⡠⠀⠀⠀⠀⠀⢀⡠⠔⠊⠁⠀⢹⣦⣤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢿⣿⣦⡀⠀⣀⣠⣴⣶⠟⢁⣀⡠⠤⠒⠈⠁⠀⠀⠀⠀⠀⢀⣿⡇⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⢿⡿⢛⣿⣿⣻⣿⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣿⣇⠸⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣠⣤⣤⣶⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠞⣿⣿⣿⣿⣆⡛⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀⣰⣿⣷⣽⣿⣿⣿⣿⡇⣨⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣿⣷⡏⣉⣵⣾⣿⣿⣿⢿⣿⣿⡿⣿⡆⠀⠀⠀⠀⠀⢀⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠙⠷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣾⣿⣿⣿⣿⡟⠁⣿⣿⣿⣇⠃⢳⠀⠀⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⠛⠿⣿⡟⠳⣤⡈⠓⢦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣿⣿⣿⣶⣾⣿⢻⣿⢹⠀⠈⡇⠀⠀⢠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣸⣿⣿⣷⣄⠉⠒⠤⣈⠙⠳⠦⣄⡀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⢀⣀⠠⢤⣶⣾⣶⣶⣦⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢣⣿⡟⡇⠀⠀⢸⡀⣠⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡏⠻⣿⣿⣿⣶⣤⣄⣉⠒⠤⣀⠉⠙⠲⢤⣀⠀⠀⠀
⢠⣴⣿⣿⠿⠊⠉⠉⢉⣭⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠟⢛⣾⡟⢱⠁⠀⠀⠀⢋⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣄⡈⠻⢿⣿⣿⣿⣿⣿⣷⣶⣭⣀⡤⣀⡈⢙⣦⣀
*/
