//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-06 20:26:51
//

#include "Model.h"
#include "Image.h"
#include "Compressor.h"
#include "Texture.h"
#include "Manager.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <MikkTSpace/mikktspace.h>

namespace MikkT
{
    int getNumFaces(const SMikkTSpaceContext* context)
    {
    	auto mesh = static_cast<ModelPrimitive*>(context->m_pUserData);
    	return static_cast<int>(mesh->IndexCount / 3);
    }

    int getNumVerticesOfFace(const SMikkTSpaceContext* context, int faceIdx)
    {
    	return 3; // We're always using triangles
    }

    void getPosition(const SMikkTSpaceContext* context, float outpos[], int faceIdx, int vertIdx)
    {
    	auto mesh = static_cast<ModelPrimitive*>(context->m_pUserData);
    	auto& pos = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Position;
    	outpos[0] = pos.x;
    	outpos[1] = pos.y;
    	outpos[2] = pos.z;
    }

    void getNormal(const SMikkTSpaceContext* context, float outnormal[], int faceIdx, int vertIdx)
    {
    	auto mesh = static_cast<ModelPrimitive*>(context->m_pUserData);
    	auto& normal = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Normal;
    	outnormal[0] = normal.x;
    	outnormal[1] = normal.y;
    	outnormal[2] = normal.z;
    }

    void getTexCoord(const SMikkTSpaceContext* context, float outuv[], int faceIdx, int vertIdx)
    {
    	auto mesh = static_cast<ModelPrimitive*>(context->m_pUserData);
    	auto& uv = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Texcoord;
    	outuv[0] = uv.x;
    	outuv[1] = uv.y;
    }

    void setTSpaceBasic(const SMikkTSpaceContext* context, const float inTangent[], float sign, int faceIdx, int vertIdx)
    {
    	auto mesh = static_cast<ModelPrimitive*>(context->m_pUserData);
    	auto& tangent = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Tangent;
    	tangent.x = inTangent[0];
    	tangent.y = inTangent[1];
    	tangent.z = inTangent[2];
    	tangent.w = -1.0f * sign;
    }
}

Model::Model(IRHIDevice* device, const String& path)
    : mParentDevice(device)
{
    mDirectory = path.substr(0, path.find_last_of('/'));

    cgltf_options options = {};
    cgltf_data* data = nullptr;

    cgltf_result result = cgltf_parse_file(&options, path.data(), &data);
    ASSERT_EQ(result == cgltf_result_success, "Failed to load model");

    result = cgltf_load_buffers(&options, data, path.data());
    ASSERT_EQ(result == cgltf_result_success, "Failed to load model data");

    cgltf_scene* scene = data->scene;

    ModelNode rootNode = {};
    rootNode.Name = "Root Node " + String(path);
    rootNode.ParentIndex = -1;
    rootNode.Transform = glm::mat4(1.0f);
    mNodes.push_back(rootNode);

    for (int i = 0; i < scene->nodes_count; i++) {
        ProcessNode(scene->nodes[i], 0);
    }
}

Model::~Model()
{
    for (auto& node : mNodes) {
        for (auto& primitive : node.Primitives) {
            delete primitive.VertexBuffer;
            delete primitive.IndexBuffer;
            delete primitive.BottomLevelAS;
        }
    }
    for (auto& material : mMaterials) {
        if (material.PBR) AssetManager::Release(material.PBR);
        if (material.Normal) AssetManager::Release(material.Normal);
        if (material.Albedo) AssetManager::Release(material.Albedo);
    }
}

void Model::ProcessNode(cgltf_node* node, int parentIndex)
{
    glm::mat4 localTransform(1.0f);
    glm::mat4 translationMatrix(1.0f);
    glm::mat4 rotationMatrix(1.0f);
    glm::mat4 scaleMatrix(1.0f);

    if (node->has_translation) {
        glm::vec3 translation = glm::vec3(node->translation[0], node->translation[1], node->translation[2]);
        translationMatrix = glm::translate(glm::mat4(1.0f), translation);
    }
    if (node->has_rotation) {
        rotationMatrix = glm::mat4_cast(glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
    }
    if (node->has_scale) {
        glm::vec3 scale = glm::vec3(node->scale[0], node->scale[1], node->scale[2]);
        scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    }

    if (node->has_matrix) {
        localTransform *= glm::make_mat4(node->matrix);
    } else {
        localTransform *= translationMatrix * rotationMatrix * scaleMatrix;
    }

    int nodeIndex = mNodes.size();

    ModelNode modelNode = {};
    modelNode.Name = node->name ? node->name : "Unnamed Node";
    modelNode.Transform = localTransform;
    modelNode.ParentIndex = parentIndex;

    if (node->mesh) {
        for (int i = 0; i < node->mesh->primitives_count; i++) {
            ProcessPrimitive(&node->mesh->primitives[i], &modelNode, localTransform);
        }
    }

    for (int i = 0; i < node->children_count; i++) {
        ProcessNode(node->children[i], modelNode.ParentIndex);
    }

    mNodes.push_back(std::move(modelNode));
}

void Model::ProcessPrimitive(cgltf_primitive* primitive, ModelNode* node, glm::mat4 localTransform)
{
    if (primitive->type != cgltf_primitive_type_triangles) {
        return;
    }

    cgltf_attribute* posAttribute = nullptr;
    cgltf_attribute* uvAttribute = nullptr;
    cgltf_attribute* normAttribute = nullptr;

    for (int i = 0; i < primitive->attributes_count; i++) {
        if (!strcmp(primitive->attributes[i].name, "POSITION")) {
            posAttribute = &primitive->attributes[i];
        }
        if (!strcmp(primitive->attributes[i].name, "TEXCOORD_0")) {
            uvAttribute = &primitive->attributes[i];
        }
        if (!strcmp(primitive->attributes[i].name, "NORMAL")) {
            normAttribute = &primitive->attributes[i];
        }
    }

    ModelPrimitive modelPrimitive = {};
    int vertexCount = posAttribute->data->count;
    int indexCount = primitive->indices->count;

    for (int i = 0; i < vertexCount; i++) {
        StaticModelVertex vertex = {};

        glm::vec3 rawPos(0.0f);
        if (!cgltf_accessor_read_float(posAttribute->data, i, glm::value_ptr(rawPos), 4)) {
            rawPos = glm::vec3(0.0f);
        }
        vertex.Position = glm::vec3(localTransform * glm::vec4(rawPos, 1.0f));
        
        if (uvAttribute) {
            if (!cgltf_accessor_read_float(uvAttribute->data, i, glm::value_ptr(vertex.Texcoord), 4)) {
                vertex.Texcoord = glm::vec2(0.0f);
            }
        } else {
            vertex.Texcoord = glm::vec2(0.0f);
        }
       
        glm::vec3 rawNormal(0.0f, 0.0f, 1.0f);
        if (!cgltf_accessor_read_float(normAttribute->data, i, glm::value_ptr(rawNormal), 4)) {
            rawNormal = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        vertex.Normal = glm::normalize(glm::mat3(glm::transpose(glm::inverse(localTransform))) * rawNormal);

        modelPrimitive.Vertices.push_back(vertex);
    }
    for (int i = 0; i < indexCount; i++) {
        modelPrimitive.Indices.push_back(cgltf_accessor_read_index(primitive->indices, i));
    }

    cgltf_material *material = primitive->material;

    ModelMaterial modelMaterial = {};
    if (material) {
        if (material->pbr_metallic_roughness.base_color_texture.texture) {
            String path = mDirectory + '/' + std::string(material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
    
            modelMaterial.Albedo = AssetManager::Get(path, AssetType::kTexture);
        }
        if (material->normal_texture.texture) {
            String path = mDirectory + '/' + std::string(material->normal_texture.texture->image->uri);
    
            modelMaterial.Normal = AssetManager::Get(path, AssetType::kTexture);
        }
        if (material->pbr_metallic_roughness.metallic_roughness_texture.texture) {
            String path = mDirectory + '/' + std::string(material->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri);
    
            modelMaterial.PBR = AssetManager::Get(path, AssetType::kTexture);
        }
    }

    modelPrimitive.VertexBuffer = mParentDevice->CreateBuffer(RHIBufferDesc(sizeof(StaticModelVertex) * vertexCount, sizeof(StaticModelVertex), RHIBufferUsage::kVertex | RHIBufferUsage::kShaderRead));
    modelPrimitive.IndexBuffer = mParentDevice->CreateBuffer(RHIBufferDesc(sizeof(uint) * indexCount, sizeof(uint), RHIBufferUsage::kIndex | RHIBufferUsage::kShaderRead));
    modelPrimitive.BottomLevelAS = mParentDevice->CreateBLAS(RHIBLASDesc(modelPrimitive.VertexBuffer, modelPrimitive.IndexBuffer));
    modelPrimitive.MaterialIndex = mMaterials.size();
    modelPrimitive.VertexCount = vertexCount;
    modelPrimitive.IndexCount = indexCount;

    SMikkTSpaceInterface mikktInterface = {};
	mikktInterface.m_getNumFaces = &MikkT::getNumFaces;
	mikktInterface.m_getNumVerticesOfFace = &MikkT::getNumVerticesOfFace;
	mikktInterface.m_getPosition = &MikkT::getPosition;
	mikktInterface.m_getNormal = &MikkT::getNormal;
	mikktInterface.m_getTexCoord = &MikkT::getTexCoord;
	mikktInterface.m_setTSpaceBasic = &MikkT::setTSpaceBasic;

	SMikkTSpaceContext context = {};
	context.m_pInterface = &mikktInterface;
	context.m_pUserData = &modelPrimitive;
	if (genTangSpaceDefault(&context) == false) {
        for (auto& vertex : modelPrimitive.Vertices) {
            vertex.Tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
	}

    Uploader::EnqueueBufferUpload(modelPrimitive.Vertices.data(), sizeof(StaticModelVertex) * vertexCount, modelPrimitive.VertexBuffer);
    Uploader::EnqueueBufferUpload(modelPrimitive.Indices.data(), sizeof(uint) * indexCount, modelPrimitive.IndexBuffer);
    Uploader::EnqueueBLASBuild(modelPrimitive.BottomLevelAS);

    modelPrimitive.Vertices.clear();
    modelPrimitive.Indices.clear();

    mMaterials.push_back(std::move(modelMaterial));
    node->Primitives.push_back(std::move(modelPrimitive));
}
