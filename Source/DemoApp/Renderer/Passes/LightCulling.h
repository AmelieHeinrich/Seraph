//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 11:55:27
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

constexpr const char* LIGHT_CULL_CLUSTER_BUFFER = "LightCulling/TileBuffer";
constexpr const char* LIGHT_CULL_CLUSTER_INDICES_BUFFER = "LightCulling/TileIndicesBuffer";
constexpr uint MAX_LIGHTS_PER_CLUSTER = 1024;
constexpr uint CLUSTER_WIDTH = 16;
constexpr uint CLUSTER_HEIGHT = 9;
constexpr uint CLUSTER_DEPTH = 8;

struct LightCluster
{
    uint Offset;
    uint Count;
    float MinDepth;
    float MaxDepth;
};

class LightCulling : public RenderPass
{
public:
    LightCulling(IRHIDevice* device, uint width, uint height);
    ~LightCulling();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;
private:
    void GenerateClusters(RenderPassBegin& begin);
    void CullClusters(RenderPassBegin& begin);

private:
    IRHIComputePipeline* mCullPipeline;

    uint mNumClustersX;
    uint mNumClustersY;
    uint mNumClustersZ;
};
