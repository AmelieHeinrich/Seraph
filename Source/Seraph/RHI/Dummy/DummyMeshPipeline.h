//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:06:50
//

#pragma once

#include <RHI/MeshPipeline.h>

class DummyDevice;

class DummyMeshPipeline : public IRHIMeshPipeline
{
public:
    DummyMeshPipeline(DummyDevice* device, RHIMeshPipelineDesc desc);
    ~DummyMeshPipeline();
};
