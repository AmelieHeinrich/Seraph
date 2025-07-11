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

    static void SubscribeGraphics(const std::string& path, const RHIGraphicsPipelineDesc& desc, const std::vector<std::string>& entryPoint);
    static void SubscribeCompute(const std::string& path, const RHIComputePipelineDesc& desc, const std::string& entryPoint);
    static void SubscribeMesh(const std::string& path, const RHIMeshPipelineDesc& desc, const std::vector<std::string>& entryPoint);

    static IRHIGraphicsPipeline* GetGraphics(const std::string& path);
    static IRHIComputePipeline* GetCompute(const std::string& path);
    static IRHIMeshPipeline* GetMesh(const std::string& path);

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
        std::string Path;
        FileTime LastWritten;
    };

    struct PipelineEntry
    {
        PipelineType Type;
        std::string ShaderFile;
        std::vector<FileWatch> Dependencies;
        std::vector<std::string> EntryPoints;

        IRHIComputePipeline* ComputePipeline;
        RHIComputePipelineDesc ComputeDesc;

        IRHIGraphicsPipeline* GraphicsPipeline;
        RHIGraphicsPipelineDesc GraphicsDesc;

        IRHIMeshPipeline* MeshPipeline;
        RHIMeshPipelineDesc MeshDesc;
    };

    static struct Data {
        IRHIDevice* Device;
        
        UnorderedMap<std::string, PipelineEntry> Entries;
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
