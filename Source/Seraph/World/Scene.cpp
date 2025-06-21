//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 18:04:43
//

#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>

Scene::Scene(IRHIDevice* device)
    : mLights(device), mDevice(device)
{
    mInstanceBuffer = device->CreateBuffer(RHIBufferDesc(sizeof(TLASInstance) * MAX_TLAS_INSTANCES, sizeof(TLASInstance), RHIBufferUsage::kConstant));
    mTLAS = device->CreateTLAS();
}

Scene::~Scene()
{
    for (auto& entity : mEntities) {
        AssetManager::Release(entity.Model);
    }
    mEntities.clear();
    
    delete mTLAS;
    delete mInstanceBuffer;
}

void Scene::Update(uint frameIndex)
{
    // Update entities
    for (auto& entity : mEntities) {
        entity.Transform = glm::translate(glm::mat4(1.0f), entity.Position)
                         * glm::toMat4(entity.Rotation)
                         * glm::scale(glm::mat4(1.0f), entity.Scale);
    }
    mLights.Update(frameIndex);

    // Update instance buffer
    mInstances.clear();
    mInstances.reserve(MAX_TLAS_INSTANCES);
    for (auto& entity : mEntities) {
        Model* model = entity.Model->Model;
        for (auto& node : model->GetNodes()) {
            for (auto& primitive : node.Primitives) {
                ModelMaterial material = model->GetMaterials()[primitive.MaterialIndex];

                TLASInstance instance = {};
                instance.AccelerationStructureReference = primitive.BottomLevelAS->GetAddress();
                instance.Transform = glm::mat3x4(glm::mat4(1.0f));
                instance.Flags = material.AlphaTested ? TLAS_INSTANCE_NON_OPAQUE : TLAS_INSTANCE_OPAQUE;
                instance.Mask = 1;
                mInstances.push_back(instance);
            }
        }
    }

    void* ptr = mInstanceBuffer->Map();
    memcpy(ptr, mInstances.data(), mInstances.size() * sizeof(TLASInstance));
    mInstanceBuffer->Unmap();
}

Entity* Scene::AddEntity(const String& modelPath)
{
    Entity entity;
    entity.Model = AssetManager::Get(modelPath, AssetType::kModel);
    mEntities.push_back(entity);
    return &mEntities.back();
}
