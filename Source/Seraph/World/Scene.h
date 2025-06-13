//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 08:03:36
//

#pragma once

#include <Renderer/Lights.h>
#include <Asset/Manager.h>

#include <glm/gtx/quaternion.hpp>

struct Entity
{
    glm::mat4 Transform;
    glm::vec3 Position;
    glm::quat Rotation;
    glm::vec3 Scale;

    Asset::Handle Model;
};

class Scene
{
public:
    Scene(IRHIDevice* device);
    ~Scene();

    void Update(uint frameIndex);
    Entity* AddEntity(const String& modelPath);

    LightList& GetLights() { return mLights; }
    Array<Entity>& GetEntities() { return mEntities; }
private:
    IRHIDevice* mDevice;

    Array<Entity> mEntities;
    LightList mLights;
};
