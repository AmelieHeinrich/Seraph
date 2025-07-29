//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:08:09
//

#include "SP_RenderWorld.h"
#include <KDAsset/KDA_MeshLoader.h>
#include <Graphics/Gfx_Material.h>
#include <Graphics/Gfx_ResourceManager.h>
#include <Graphics/Gfx_ViewRecycler.h>
#include <KernelCore/KC_Hash.h>

namespace SP
{
    RenderWorld::RenderWorld()
    {
        mEntities.reserve(RENDER_WORLD_MAX_INSTANCES);
        mMaterials.reserve(RENDER_WORLD_MAX_MATERIALS);
        mRTWorld = KC_NEW(Gfx::RaytracingWorld);
        mLightList = KC_NEW(LightList);

        mSceneInstanceBuffer = Gfx::Manager::GetDevice()->CreateBuffer(KGPU::BufferDesc(sizeof(SceneInstance) * RENDER_WORLD_MAX_INSTANCES, sizeof(SceneInstance), KGPU::BufferUsage::kShaderRead | KGPU::BufferUsage::kStaging));
        mSceneMaterialBuffer = Gfx::Manager::GetDevice()->CreateBuffer(KGPU::BufferDesc(sizeof(SceneMaterial) * RENDER_WORLD_MAX_MATERIALS, sizeof(SceneMaterial), KGPU::BufferUsage::kShaderRead | KGPU::BufferUsage::kStaging));
    }

    RenderWorld::~RenderWorld()
    {
        // First delete all primitives
        for (auto& [path, entityList] : mEntities) {
            for (auto& entity : entityList) {
                KC_DELETE(entity.Primitive);
            }
        }
        mEntities.clear();

        // Then delete all materials
        for (auto& material : mMaterials) {
            if (material.Material) {
                KC_DELETE(material.Material);
            }
        }
        mMaterials.clear();

        KC_DELETE(mSceneInstanceBuffer);
        KC_DELETE(mSceneMaterialBuffer);
        KC_DELETE(mLightList);
        KC_DELETE(mRTWorld);
    }

    void RenderWorld::AddMesh(const KC::String& path)
    {
        if (mEntities.contains(path))
            return;

        auto mesh = KDA::MeshLoader::LoadMeshFromFile(path);
        for (int i = 0; i < mesh.Nodes.size(); i++) {
            for (int x = 0; x < mesh.Nodes[i].Primitives.size(); x++) {
                auto& prim = mesh.Nodes[i].Primitives[x];

                RenderEntity entity = {};
                entity.MaterialIndex = FindOrAddMaterial(mesh.Materials[prim.MaterialIndex]);
                entity.Primitive = KC_NEW(Gfx::MeshPrimitive, prim);

                mEntities[path].push_back(entity);
            }
        }
    }

    void RenderWorld::RemoveMesh(const KC::String& path)
    {
        auto it = mEntities.find(path);
        if (it == mEntities.end())
            return;

        for (auto& entity : it->second) {
            RenderMaterial& material = mMaterials[entity.MaterialIndex];
            material.ReferenceCount--;
            if (material.ReferenceCount == 0) {
                KC_DELETE(material.Material);
            }

            KC_DELETE(entity.Primitive);
        }
        mEntities.erase(it);
    }

    uint64 RenderWorld::HashMaterial(const KDA::MeshMaterial& material)
    {
        uint64 hash = 0;
        hash = KC::HashCombine(hash, KC::Hash(material.AlbedoPath));
        hash = KC::HashCombine(hash, KC::Hash(material.NormalPath));
        hash = KC::HashCombine(hash, KC::Hash(material.MetallicRoughnessPath));
        hash = KC::HashCombine(hash, KC::Hash(material.EmissivePath));
        return hash;
    }

    int RenderWorld::FindOrAddMaterial(const KDA::MeshMaterial& material)
    {
        size_t hash = HashMaterial(material);
        
        // Find existing material
        for (int i = 0; i < mMaterials.size(); i++) {
            if (mMaterials[i].Hash == hash) {
                mMaterials[i].ReferenceCount++;
                return i;
            }
        }
        
        // Create new material
        RenderMaterial renderMat;
        renderMat.Hash = hash;
        renderMat.ReferenceCount = 1;
        renderMat.Material = KC_NEW(Gfx::Material, material);
        
        mMaterials.push_back(renderMat);
        return mMaterials.size() - 1;
    }

    void RenderWorld::ForEach(const std::function<void(RenderEntity entity, Gfx::Material* material)>& function)
    {
        for (const auto& [path, entityList] : mEntities) {
            for (auto& entity : entityList) {
                function(entity, mMaterials[entity.MaterialIndex].Material);
            }
        }
    }

    void RenderWorld::Update()
    {
        // Reset world
        mRTWorld->Reset();
        mSceneInstances.clear();
        mSceneMaterials.clear();

        for (const auto& [path, entityList] : mEntities) {
            for (auto& entity : entityList) {
                auto material = mMaterials[entity.MaterialIndex].Material;
                mRTWorld->AddInstance(entity.Primitive, KGPU::float4x4(1.0f), material->IsOpaque());
                
                SceneInstance instance;
                instance.VertexBuffer = entity.Primitive->GetVertexBufferView()->GetBindlessHandle();
                instance.IndexBuffer = entity.Primitive->GetIndexBufferView()->GetBindlessHandle();
                instance.MaterialIndex = entity.MaterialIndex;
                mSceneInstances.push_back(instance);
            }
        }

        Gfx::Resource& defaultWhite = Gfx::ResourceManager::Get(Gfx::DEFAULT_WHITE_TEXTURE);
        for (auto& material : mMaterials) {
            SceneMaterial sceneMat;
            sceneMat.AlbedoTexture = material.Material->GetAlbedoView() ? material.Material->GetAlbedoView()->GetBindlessHandle() : Gfx::ViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();
            sceneMat.NormalTexture = material.Material->GetNormalView() ? material.Material->GetNormalView()->GetBindlessHandle() : KGPU::BINDLESS_INVALID_HANDLE;
            sceneMat.PBRTexture = material.Material->GetMRView() ? material.Material->GetMRView()->GetBindlessHandle() : KGPU::BINDLESS_INVALID_HANDLE;
            sceneMat.EmissiveTexture = material.Material->GetEmissiveView() ? material.Material->GetEmissiveView()->GetBindlessHandle() : KGPU::BINDLESS_INVALID_HANDLE;
        
            mSceneMaterials.push_back(sceneMat);
        }

        void* ptr = mSceneInstanceBuffer->Map();
        memcpy(ptr, mSceneInstances.data(), mSceneInstances.size() * sizeof(SceneInstance));
        mSceneInstanceBuffer->Unmap();

        ptr = mSceneMaterialBuffer->Map();
        memcpy(ptr, mSceneMaterials.data(), mSceneMaterials.size() * sizeof(SceneMaterial));
        mSceneMaterialBuffer->Unmap();
    }
}
