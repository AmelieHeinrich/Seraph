//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 16:33:42
//

#pragma once

#include <KernelCore/KC_Context.h>
#include <KernelGPU/KGPU_ShaderCompat.h>

namespace KDA
{
    struct MeshVertex
    {
        KGPU::float3 Position;
        KGPU::float3 Normal;
        KGPU::float4 Tangent;
        KGPU::float2 UV;
    };

    struct MeshPrimitive
    {
        KC::Array<MeshVertex> Vertices;
        KC::Array<uint> Indices;

        uint MaterialIndex;
    };

    struct MeshNode
    {
        int ParentIndex;

        KC::Array<MeshPrimitive> Primitives;
        KC::String Name;
    };

    struct MeshMaterial
    {
        bool AlphaTested = false;
        float AlphaCutout = 0.5f;

        KC::String AlbedoPath;
        KC::String NormalPath;
        KC::String MetallicRoughnessPath;
        KC::String EmissivePath;
    };

    struct Mesh
    {
        bool IsSkinned = false;
        KC::String ParentDirectory;

        KC::Array<MeshNode> Nodes;
        KC::Array<MeshMaterial> Materials;
    };

    class MeshLoader
    {
    public:
        static Mesh LoadMeshFromFile(const KC::String& path);
    };
}
