//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 12:50:49
//

#pragma once

#include <Graphics/Gfx_MeshPrimitive.h>
#include <Graphics/Gfx_Material.h>
#include <Graphics/Gfx_RaytracingWorld.h>
#include <KDAsset/KDA_MeshLoader.h>

#include <functional>

#include "SP_Lights.h"

namespace SP
{
    constexpr uint RENDER_WORLD_MAX_INSTANCES = 4192;
    constexpr uint RENDER_WORLD_MAX_MATERIALS = 4192;

    struct RenderEntity
    {
        uint MaterialIndex;
        Gfx::MeshPrimitive* Primitive;
    };

    struct SceneInstance
    {
        KGPU::BindlessHandle VertexBuffer;
        KGPU::BindlessHandle IndexBuffer;
        uint MaterialIndex;
        uint Pad;
    };

    struct SceneMaterial
    {
        KGPU::BindlessHandle AlbedoTexture;
        KGPU::BindlessHandle NormalTexture;
        KGPU::BindlessHandle PBRTexture;
        KGPU::BindlessHandle EmissiveTexture;
    };

    struct RenderMaterial
    {
        int ReferenceCount = 0;
        uint64 Hash = 0;
        Gfx::Material* Material;
    };

    class RenderWorld
    {
    public:
        RenderWorld();
        ~RenderWorld();

        void AddMesh(const KC::String& path);
        void RemoveMesh(const KC::String& path);

        void ForEach(const std::function<void(RenderEntity entity, Gfx::Material* material)>& function);
        
        Gfx::RaytracingWorld* GetRTWorld() { return mRTWorld; }   
        LightList* GetLightList() { return mLightList; }

        void Update();
        KGPU::IBuffer* GetSceneInstanceBuffer() { return mSceneInstanceBuffer; }
        KGPU::IBuffer* GetSceneMaterialBuffer() { return mSceneMaterialBuffer; }
    private:
        uint64 HashMaterial(const KDA::MeshMaterial& material);
        int FindOrAddMaterial(const KDA::MeshMaterial& material);

    private:
        KC::HashMap<KC::String, KC::Array<int>> mModelToMeshIndices;
        KC::HashMap<KC::String, KC::Array<RenderEntity>> mEntities;
        KC::Array<RenderMaterial> mMaterials;

        KC::Array<SceneInstance> mSceneInstances;
        KGPU::IBuffer* mSceneInstanceBuffer;
        KC::Array<SceneMaterial> mSceneMaterials;
        KGPU::IBuffer* mSceneMaterialBuffer;

        Gfx::RaytracingWorld* mRTWorld;
        LightList* mLightList;
    };
}
