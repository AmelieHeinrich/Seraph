//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 18:04:43
//

#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Renderer/RendererViewRecycler.h>

Scene::Scene(IRHIDevice* device)
    : mLights(device), mDevice(device)
{
    mInstanceBuffer = device->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance) * MAX_TLAS_INSTANCES, sizeof(TLASInstance), RHIBufferUsage::kConstant));
    mTLAS = device->CreateTLAS();

    mSceneInstances = device->CreateBuffer(RHIBufferDesc(sizeof(SceneInstance) * MAX_TLAS_INSTANCES, sizeof(SceneInstance), RHIBufferUsage::kShaderRead | RHIBufferUsage::kConstant));
    mSceneMaterials = device->CreateBuffer(RHIBufferDesc(sizeof(SceneMaterial) * MAX_TLAS_INSTANCES, sizeof(SceneMaterial), RHIBufferUsage::kShaderRead | RHIBufferUsage::kConstant));
}

Scene::~Scene()
{
    for (auto& entity : mEntities) {
        AssetManager::Release(entity.Model);
    }
    mEntities.clear();
    
    delete mTLAS;
    delete mInstanceBuffer;
    delete mSceneInstances;
    delete mSceneMaterials;
}

void Scene::Update(uint frameIndex)
{
    // Update entity transforms
    for (auto& entity : mEntities) {
        entity.Transform = glm::translate(glm::mat4(1.0f), entity.Position)
                         * glm::toMat4(entity.Rotation)
                         * glm::scale(glm::mat4(1.0f), entity.Scale);
    }

    mLights.Update(frameIndex);

    // Material deduplication map
    std::unordered_map<size_t, uint> materialMap;
    Array<SceneMaterial> materials;
    Array<SceneInstance> instances;

    mInstances.clear();
    mInstances.reserve(MAX_TLAS_INSTANCES);

    for (auto& entity : mEntities) {
        Model* model = entity.Model->Model;
        for (auto& node : model->GetNodes()) {
            for (auto& primitive : node.Primitives) {
                const ModelMaterial& mat = model->GetMaterials()[primitive.MaterialIndex];

                // Create a unique hash for the material
                size_t hash = std::hash<uint64>{}((uint64)mat.Albedo) ^
                              std::hash<uint64>{}((uint64)mat.Normal) ^
                              std::hash<uint64>{}((uint64)mat.PBR);

                uint materialIndex;
                auto it = materialMap.find(hash);
                if (it == materialMap.end()) {
                    materialIndex = static_cast<uint>(materials.size());
                    materialMap[hash] = materialIndex;

                    SceneMaterial sceneMat = {
                        mat.Albedo ? mat.Albedo->TextureOrImage.View->GetBindlessHandle() : BindlessHandle{},
                        mat.Normal ? mat.Normal->TextureOrImage.View->GetBindlessHandle() : BindlessHandle{},
                        mat.PBR    ? mat.PBR->TextureOrImage.View->GetBindlessHandle()    : BindlessHandle{},
                        0
                    };
                    materials.push_back(sceneMat);
                } else {
                    materialIndex = it->second;
                }

                TLASInstance instance = {};
                instance.AccelerationStructureReference = primitive.BottomLevelAS->GetAddress();
                instance.Transform = glm::mat3x4(entity.Transform);
                instance.Flags = mat.AlphaTested ? TLAS_INSTANCE_NON_OPAQUE : TLAS_INSTANCE_OPAQUE;
                instance.Mask = 1;
                mInstances.push_back(instance);

                SceneInstance sceneInstance = {
                    RendererViewRecycler::GetSRV(primitive.VertexBuffer)->GetBindlessHandle(),
                    RendererViewRecycler::GetSRV(primitive.IndexBuffer)->GetBindlessHandle(),
                    materialIndex,
                    0
                };
                instances.push_back(sceneInstance);
            }
        }
    }

    // Upload TLAS instances
    void* ptr = mInstanceBuffer->Map();
    memcpy(ptr, mInstances.data(), mInstances.size() * sizeof(TLASInstance));
    mInstanceBuffer->Unmap();

    // Upload SceneInstance data
    ptr = mSceneInstances->Map();
    memcpy(ptr, instances.data(), instances.size() * sizeof(SceneInstance));
    mSceneInstances->Unmap();

    // Upload SceneMaterial data
    ptr = mSceneMaterials->Map();
    memcpy(ptr, materials.data(), materials.size() * sizeof(SceneMaterial));
    mSceneMaterials->Unmap();
}

Entity* Scene::AddEntity(const String& modelPath)
{
    Entity entity;
    entity.Model = AssetManager::Get(modelPath, AssetType::kModel);
    mEntities.push_back(entity);
    return &mEntities.back();
}
