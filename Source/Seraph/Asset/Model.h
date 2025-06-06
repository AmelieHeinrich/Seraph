//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-06 20:20:26
//

#pragma once

#include <glm/glm.hpp>
#include <CGLTF/cgltf.h>

#include <RHI/Device.h>
#include <RHI/Uploader.h>

struct StaticModelVertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 Texcoord;
};

struct ModelMaterial
{
    bool AlphaTested = false;

    IRHITexture* Texture;
    IRHITextureView* TextureRead;
};

struct ModelPrimitive
{
    IRHIBuffer* VertexBuffer;
    IRHIBuffer* IndexBuffer;
    IRHIBLAS* BottomLevelAS;

    uint VertexCount;
    uint IndexCount;
    uint MaterialIndex;
};

struct ModelNode
{
    int ParentIndex;
    glm::mat4 Transform;
    Array<ModelPrimitive> Primitives;
    String Name;
};

class Model
{
public:
    Model(IRHIDevice* device, const StringView& path);
    ~Model();

    Array<ModelNode>& GetNodes() { return mNodes; }
    Array<ModelMaterial>& GetMaterials() { return mMaterials; }
private:
    void ProcessNode(cgltf_node* node, int parentIndex);
    void ProcessPrimitive(cgltf_primitive* primitive, ModelNode* node, glm::mat4 localTransform);

    IRHIDevice* mParentDevice;
    String mDirectory;

    Array<ModelNode> mNodes;
    Array<ModelMaterial> mMaterials;
};
