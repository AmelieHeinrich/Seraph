//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 16:43:18
//

#include "KDA_MeshLoader.h"
#include "KDA_TangentCalculator.h"

#include <cgltf.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace KDA
{
    using namespace KGPU;

    void LoadMeshPrimitive(Mesh& mesh, cgltf_primitive* primitive, MeshNode* node, float4x4 localTransform)
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

        MeshPrimitive meshPrimitive = {};
        int vertexCount = posAttribute->data->count;
        int indexCount = primitive->indices->count;

        for (int i = 0; i < vertexCount; i++) {
            MeshVertex vertex = {};

            float3 rawPos(0.0f);
            if (!cgltf_accessor_read_float(posAttribute->data, i, glm::value_ptr(rawPos), 4)) {
                rawPos = float3(0.0f);
            }
            vertex.Position = float3(localTransform * float4(rawPos, 1.0f));

            if (uvAttribute) {
                if (!cgltf_accessor_read_float(uvAttribute->data, i, glm::value_ptr(vertex.UV), 4)) {
                    vertex.UV = float2(0.0f);
                }
            } else {
                vertex.UV = float2(0.0f);
            }
        
            float3 rawNormal(0.0f, 0.0f, 1.0f);
            if (!cgltf_accessor_read_float(normAttribute->data, i, glm::value_ptr(rawNormal), 4)) {
                rawNormal = float3(0.0f, 0.0f, 1.0f);
            }
            vertex.Normal = glm::normalize(glm::mat3(glm::transpose(glm::inverse(localTransform))) * rawNormal);

            meshPrimitive.Vertices.push_back(vertex);
        }
        
        for (int i = 0; i < indexCount; i++) {
            meshPrimitive.Indices.push_back(cgltf_accessor_read_index(primitive->indices, i));
        }

        TangentCalculator::ComputeTangents(&meshPrimitive);

        cgltf_material* material = primitive->material;

        MeshMaterial meshMaterial = {};
        if (material) {
            if (material->pbr_metallic_roughness.base_color_texture.texture) {
                meshMaterial.AlbedoPath = mesh.ParentDirectory + '/' + KC::String(material->pbr_metallic_roughness.base_color_texture.texture->image->uri);
            }
            if (material->normal_texture.texture) {
                meshMaterial.NormalPath = mesh.ParentDirectory + '/' + KC::String(material->normal_texture.texture->image->uri);
            }
            if (material->pbr_metallic_roughness.metallic_roughness_texture.texture) {
                meshMaterial.MetallicRoughnessPath = mesh.ParentDirectory + '/' + KC::String(material->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri);
            }
            if (material->emissive_texture.texture) {
                meshMaterial.EmissivePath = mesh.ParentDirectory + '/' + KC::String(material->emissive_texture.texture->image->uri);
            }
            if (material->alpha_mode != cgltf_alpha_mode_opaque) {
                meshMaterial.AlphaTested = true;
                meshMaterial.AlphaCutout = material->alpha_cutoff;
            }
        }

        meshPrimitive.MaterialIndex = mesh.Materials.size();
        mesh.Materials.push_back(meshMaterial);
        node->Primitives.push_back(meshPrimitive);
    }

    void LoadMeshNode(Mesh& mesh, cgltf_node* node, int parentIndex, glm::mat4 transform)
    {
        float4x4 localTransform = transform;
        float4x4 translationMatrix(1.0f);
        float4x4 rotationMatrix(1.0f);
        float4x4 scaleMatrix(1.0f);

        if (node->has_translation) {
            float3 translation = float3(node->translation[0], node->translation[1], node->translation[2]);
            translationMatrix = glm::translate(float4x4(1.0f), translation);
        }
        if (node->has_rotation) {
            rotationMatrix = glm::mat4_cast(glm::quat(node->rotation[3], node->rotation[0], node->rotation[1], node->rotation[2]));
        }
        if (node->has_scale) {
            float3 scale = float3(node->scale[0], node->scale[1], node->scale[2]);
            scaleMatrix = glm::scale(float4x4(1.0f), scale);
        }

        if (node->has_matrix) {
            localTransform *= glm::make_mat4(node->matrix);
        } else {
            localTransform *= translationMatrix * rotationMatrix * scaleMatrix;
        }

        int nodeIndex = mesh.Nodes.size();

        MeshNode meshNode = {};
        meshNode.Name = node->name ? node->name : "Unnamed Node";
        meshNode.ParentIndex = parentIndex;

        if (node->mesh) {
            for (int i = 0; i < node->mesh->primitives_count; i++) {
                LoadMeshPrimitive(mesh, &node->mesh->primitives[i], &meshNode, localTransform);
            }
        }

        for (int i = 0; i < node->children_count; i++) {
            LoadMeshNode(mesh, node->children[i], meshNode.ParentIndex, localTransform);
        }

        mesh.Nodes.push_back(std::move(meshNode));
    }

    Mesh MeshLoader::LoadMeshFromFile(const KC::String& path)
    {
        Mesh result;
        result.IsSkinned = false;
        result.ParentDirectory = path.substr(0, path.find_last_of('/'));

        cgltf_options options = {};
        cgltf_data* data = nullptr;

        cgltf_result err = cgltf_parse_file(&options, path.data(), &data);
        if (err != cgltf_result_success) {
            KD_ERROR("Failed to parse GLTF file %s", path.c_str());
            return {};
        }

        err = cgltf_load_buffers(&options, data, path.data());
        if (err != cgltf_result_success) {
            KD_ERROR("Failed to load GLTF file %s", path.c_str());
            return {};
        }

        cgltf_scene* scene = data->scene;

        MeshNode rootNode = {};
        rootNode.Name = "Root Node " + std::string(path);
        rootNode.ParentIndex = -1;
        result.Nodes.push_back(rootNode);

        for (int i = 0; i < scene->nodes_count; i++) {
            LoadMeshNode(result, scene->nodes[i], 0, glm::mat4(1.0f));
        }

        return result;
    }
}
