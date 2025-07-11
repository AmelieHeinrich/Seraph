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
    float3 Position = glm::vec3(0.0f);
    glm::quat Rotation = glm::identity<glm::quat>();
    float3 Scale = glm::vec3(1.0f);

    Asset::Handle Model;
};

struct SceneInstance
{
    BindlessHandle VertexBuffer;
    BindlessHandle IndexBuffer;
    uint MaterialIndex;
    BindlessHandle MaterialBuffer;
};

class Scene
{
public:
    Scene(IRHIDevice* device);
    ~Scene();

    void Update(uint frameIndex);
    Entity* AddEntity(const std::string& modelPath);

    LightList& GetLights() { return mLights; }
    std::vector<Entity>& GetEntities() { return mEntities; }

    std::vector<TLASInstance>& GetTLASInstances() { return mInstances; }
    IRHITLAS* GetTLAS() { return mTLAS; }
    IRHIBuffer* GetInstanceBuffer() { return mInstanceBuffer; }

    IRHIBuffer* GetSceneInstanceBuffer() { return mSceneInstances; }
private:
    IRHIDevice* mDevice;

    std::vector<Entity> mEntities;
    LightList mLights;

    std::vector<TLASInstance> mInstances;
    IRHIBuffer* mInstanceBuffer;
    IRHITLAS* mTLAS;

    IRHIBuffer* mSceneInstanceTransfer;
    IRHIBuffer* mSceneInstances;
};
