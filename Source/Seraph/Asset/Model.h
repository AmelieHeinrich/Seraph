//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-06 20:20:26
//

#pragma once

#include <glm/glm.hpp>
#include <CGLTF/cgltf.h>

#include <RHI/Device.h>
#include <RHI/Uploader.h>

class Asset;

struct alignas(16) StaticModelVertex
{
    float3 Position;   // 12 bytes
    float _pad0;          // 4 bytes padding

    float3 Normal;     // 12 bytes
    float _pad1;          // 4 bytes padding

    float2 Texcoord;   // 8 bytes
    float _pad2[2];       // 8 bytes padding

    float4 Tangent;
};

struct ModelMaterial
{
    bool AlphaTested = false;

    Asset* Albedo;
    Asset* Normal;
    Asset* PBR;
};

struct ModelPrimitive
{
    IRHIBuffer* VertexBuffer;
    IRHIBuffer* IndexBuffer;
    IRHIBLAS* BottomLevelAS;

    Array<StaticModelVertex> Vertices;
    Array<uint> Indices;

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

struct SceneMaterial
{
    BindlessHandle AlbedoIndex;
    BindlessHandle NormalIndex;
    BindlessHandle PBRIndex;
    uint Pad;
};

class Model
{
public:
    Model(IRHIDevice* device, const String& path);
    ~Model();

    Array<ModelNode>& GetNodes() { return mNodes; }
    Array<ModelMaterial>& GetMaterials() { return mMaterials; }

    IRHIBuffer* GetMaterialBuffer() { return mMaterialBuffer; }
private:
    void ProcessNode(cgltf_node* node, int parentIndex);
    void ProcessPrimitive(cgltf_primitive* primitive, ModelNode* node, glm::mat4 localTransform);

    IRHIBuffer* mMaterialBuffer;

    IRHIDevice* mParentDevice;
    String mDirectory;

    Array<ModelNode> mNodes;
    Array<ModelMaterial> mMaterials;
};
