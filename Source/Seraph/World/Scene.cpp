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
}

Scene::~Scene()
{
    for (auto& entity : mEntities) {
        AssetManager::Release(entity.Model);
    }
    mEntities.clear();
}

void Scene::Update()
{
    for (auto& entity : mEntities) {
        entity.Transform = glm::translate(glm::mat4(1.0f), entity.Position)
                         * glm::toMat4(entity.Rotation)
                         * glm::scale(glm::mat4(1.0f), entity.Scale);
    }
}

Entity* Scene::AddEntity(const String& modelPath)
{
    Entity entity;
    entity.Model = AssetManager::Get(modelPath, AssetType::kModel);
    mEntities.push_back(entity);
    return &mEntities.back();
}
