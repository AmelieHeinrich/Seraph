//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-07 14:55:31
//

#include "GBuffer.h"

#include <imgui/imgui.h>
#include <Seraph/Math/PerlinNoise.h>

float smoothstep(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float SampleHeight(float fx, float fz, const PerlinNoise& noise)
{
    float fbm = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.01f;
    for (int i = 0; i < 8; ++i) {  // 32 might be overkill; fewer octaves for smoother base
        fbm += noise.Noise(fx * frequency, fz * frequency) * amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    // Normalize fbm to [0,1] assuming fbm roughly in [-1,1] (depends on your noise)
    fbm = fbm * 0.5f + 0.5f;

    // Flatten low values, keep high values sharp:
    // This smoothstep compresses low values near 0, keeps peaks near 1
    float flatFbm = smoothstep(0.2f, 0.8f, fbm);
    return (flatFbm * 40.0f) - 30.0f;
}

GBuffer::GBuffer(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    // Textures
    RHITextureDesc depthDesc, normalDesc, albedoDesc, pbrDesc;
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.Format = RHITextureFormat::kD32_FLOAT;
    depthDesc.Usage = RHITextureUsage::kDepthTarget | RHITextureUsage::kShaderResource;

    normalDesc.Width = width;
    normalDesc.Height = height;
    normalDesc.Format = RHITextureFormat::kR16G16B16A16_FLOAT;
    normalDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    albedoDesc.Width = width;
    albedoDesc.Height = height;
    albedoDesc.Format = RHITextureFormat::kR8G8B8A8_UNORM;
    albedoDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    pbrDesc.Width = width;
    pbrDesc.Height = height;
    pbrDesc.Format = RHITextureFormat::kR16G16_FLOAT;
    pbrDesc.Usage = RHITextureUsage::kRenderTarget | RHITextureUsage::kShaderResource;

    RendererResourceManager::CreateTexture(GBUFFER_DEPTH_ID, depthDesc);
    RendererResourceManager::CreateTexture(GBUFFER_NORMAL_ID, normalDesc);
    RendererResourceManager::CreateTexture(GBUFFER_ALBEDO_ID, albedoDesc);
    RendererResourceManager::CreateTexture(GBUFFER_PBR_ID, pbrDesc);
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kLinear, true));
    RendererResourceManager::CreateSampler(GBUFFER_DEFAULT_NEAREST_SAMPLER_ID, RHISamplerDesc(RHISamplerAddress::kWrap, RHISamplerFilter::kNearest, true));

    // Shader
    CompiledShader shader = ShaderCompiler::Compile("GBuffer", { "VSMain", "FSMain" });

    RHIGraphicsPipelineDesc pipelineDesc = {};
    pipelineDesc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    pipelineDesc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    pipelineDesc.PushConstantSize = sizeof(BindlessHandle) * 8 + sizeof(glm::mat4) * 2;
    pipelineDesc.RenderTargetFormats = {
        normalDesc.Format,
        albedoDesc.Format,
        pbrDesc.Format
    };
    pipelineDesc.DepthEnabled = true;
    pipelineDesc.DepthWrite = true;
    pipelineDesc.DepthFormat = RHITextureFormat::kD32_FLOAT;
    pipelineDesc.DepthOperation = RHIDepthOperation::kLess;
    pipelineDesc.FillMode = RHIFillMode::kSolid;
    pipelineDesc.CullMode = RHICullMode::kBack;

    mPipeline = mParentDevice->CreateGraphicsPipeline(pipelineDesc);

    pipelineDesc.FillMode = RHIFillMode::kWireframe;
    mWirePipeline = mParentDevice->CreateGraphicsPipeline(pipelineDesc);

    // Camera CBV
    RendererResourceManager::CreateRingBuffer(GBUFFER_CAMERA_CBV_ID, Align<uint>(sizeof(CameraData), 256));

    // Terrain
    CODE_BLOCK("Generate Terrain") {
        const int gridSize = 1024;
        const float spacing = 0.125f;

        int halfGrid = gridSize / 2;
        float originX = std::floor(0.0f / spacing) * spacing - halfGrid * spacing;
        float originZ = std::floor(-1.0f / spacing) * spacing - halfGrid * spacing;

        PerlinNoise noise(time(nullptr));
        Array<StaticModelVertex> vertices;
        for (int z = 0; z < gridSize; ++z) {
            for (int x = 0; x < gridSize; ++x) {
                float fx = originX + x * spacing;
                float fz = originZ + z * spacing;

                fx = round(fx / spacing) * spacing;
                fz = round(fz / spacing) * spacing;

                float height = SampleHeight(fx, fz, noise);
                float detail = noise.Noise(fx * 0.2f, fz * 0.2f) * 0.5f + 0.5f;
                height += (detail - 0.5f) * 1.0f;

                float hL = SampleHeight(fx - spacing, fz, noise);
                float hR = SampleHeight(fx + spacing, fz, noise);
                if (x == 0) hL = height;
                if (x == gridSize - 1) hR = height;

                float hD = SampleHeight(fx, fz - spacing, noise);
                float hU = SampleHeight(fx, fz + spacing, noise);
                if (z == 0) hD = height;
                if (z == gridSize - 1) hU = height;
            
                glm::vec3 normal = glm::normalize(glm::vec3(hL - hR, 2.0f * spacing, hD - hU));

                StaticModelVertex vertex;
                vertex.Position = { fx, height, fz };
                vertex.Normal = normal;
                vertex.Texcoord = { (float)x / (gridSize - 1), (float)z / (gridSize - 1) };
                vertex.Tangent = glm::vec4(0);
            
                vertices.push_back(vertex);
            }
        }

        Array<uint> indices;
        for (int z = 0; z < gridSize - 1; ++z) {
            for (int x = 0; x < gridSize - 1; ++x) {
                int topLeft     = x + z * gridSize;
                int topRight    = (x + 1) + z * gridSize;
                int bottomLeft  = x + (z + 1) * gridSize;
                int bottomRight = (x + 1) + (z + 1) * gridSize;
            
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);
            
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        for (size_t i = 0; i < indices.size(); i += 3) {
            uint i0 = indices[i];
            uint i1 = indices[i + 1];
            uint i2 = indices[i + 2];

            glm::vec3 v0 = vertices[i0].Position;
            glm::vec3 v1 = vertices[i1].Position;
            glm::vec3 v2 = vertices[i2].Position;

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            vertices[i0].Normal += faceNormal;
            vertices[i1].Normal += faceNormal;
            vertices[i2].Normal += faceNormal;
        }

        // Smooth normals
        for (auto& vertex : vertices) {
            vertex.Normal = glm::normalize(vertex.Normal);
        }

        mTerrainVB = mParentDevice->CreateBuffer(RHIBufferDesc(vertices.size() * sizeof(StaticModelVertex), sizeof(StaticModelVertex), RHIBufferUsage::kShaderRead));
        mTerrainIB = mParentDevice->CreateBuffer(RHIBufferDesc(indices.size() * sizeof(uint), sizeof(uint), RHIBufferUsage::kIndex));

        Uploader::EnqueueBufferUpload(vertices.data(), mTerrainVB->GetDesc().Size, mTerrainVB);
        Uploader::EnqueueBufferUpload(indices.data(), mTerrainIB->GetDesc().Size, mTerrainIB);
    }
}

GBuffer::~GBuffer()
{
    delete mPipeline;
}

void GBuffer::Render(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("GBuffer");
    {
        RendererResource& cameraBuffer = RendererResourceManager::Get(GBUFFER_CAMERA_CBV_ID);
        void* ptr = cameraBuffer.RingBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, &begin.CamData, sizeof(begin.CamData));
        cameraBuffer.RingBuffer[begin.FrameIndex]->Unmap();

        RendererResource& depthTexture = RendererResourceManager::Import(GBUFFER_DEPTH_ID, begin.CommandList, RendererImportType::kDepthWrite);
        RendererResource& normalTexture = RendererResourceManager::Import(GBUFFER_NORMAL_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& albedoTexture = RendererResourceManager::Import(GBUFFER_ALBEDO_ID, begin.CommandList, RendererImportType::kColorWrite);
        RendererResource& pbrTexture = RendererResourceManager::Import(GBUFFER_PBR_ID, begin.CommandList, RendererImportType::kColorWrite);

        RendererResource& materialSampler = RendererResourceManager::Get(GBUFFER_DEFAULT_MATERIAL_SAMPLER_ID);
        RendererResource& defaultWhite = RendererResourceManager::Get(DEFAULT_WHITE_TEXTURE);

        Array<RHIRenderAttachment> attachments = {
            RHIRenderAttachment(RendererViewRecycler::GetRTV(normalTexture.Texture)),
            RHIRenderAttachment(RendererViewRecycler::GetRTV(albedoTexture.Texture)),
            RHIRenderAttachment(RendererViewRecycler::GetRTV(pbrTexture.Texture))
        };
        RHIRenderBegin renderBegin(mWidth, mHeight, attachments, RHIRenderAttachment(RendererViewRecycler::GetDSV(depthTexture.Texture)));
    
        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetGraphicsPipeline(mWireframe ? mWirePipeline : mPipeline);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);

        // Render terrain
        CODE_BLOCK("Draw Terrain") {
            struct PushConstant {
                BindlessHandle Albedo;
                BindlessHandle Normal;
                BindlessHandle PBR;
                BindlessHandle Sampler;
            
                BindlessHandle VertexBuffer;
                uint pad[3];

                glm::mat4 View;
                glm::mat4 Projection;
            } constant = {
                RendererViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle(),
                INVALID_HANDLE,
                INVALID_HANDLE,
                materialSampler.Sampler->GetBindlessHandle(),

                RendererViewRecycler::GetSRV(mTerrainVB)->GetBindlessHandle(),
                {0,0,0},

                begin.CamData.View,
                begin.CamData.Proj
            };
            
            begin.CommandList->SetIndexBuffer(mTerrainIB);
            begin.CommandList->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
            begin.CommandList->DrawIndexed(mTerrainIB->GetDesc().Size / sizeof(uint), 1, 0, 0, 0);
        }

        // Render meshes
        for (auto& entity : begin.RenderScene->GetEntities()) {
            Model* model = entity.Model->Model;
            for (auto& node : model->GetNodes()) {
                for (auto& primitive : node.Primitives) {
                    ModelMaterial material = model->GetMaterials()[primitive.MaterialIndex];
                    BindlessHandle albedoView = material.Albedo ? material.Albedo->TextureOrImage.View->GetBindlessHandle() : RendererViewRecycler::GetSRV(defaultWhite.Texture)->GetBindlessHandle();
                    BindlessHandle normalView = material.Normal ? material.Normal->TextureOrImage.View->GetBindlessHandle() : INVALID_HANDLE;
                    BindlessHandle pbrView = material.PBR ? material.PBR->TextureOrImage.View->GetBindlessHandle() : INVALID_HANDLE;

                    struct PushConstant {
                        BindlessHandle Albedo;
                        BindlessHandle Normal;
                        BindlessHandle PBR;
                        BindlessHandle Sampler;
                    
                        BindlessHandle VertexBuffer;
                        uint pad[3];

                        glm::mat4 View;
                        glm::mat4 Projection;
                    } constant = {
                        albedoView,
                        normalView,
                        pbrView,
                        materialSampler.Sampler->GetBindlessHandle(),

                        RendererViewRecycler::GetSRV(primitive.VertexBuffer)->GetBindlessHandle(),
                        {0,0,0},

                        begin.CamData.View,
                        begin.CamData.Proj
                    };
                
                    begin.CommandList->SetIndexBuffer(primitive.IndexBuffer);
                    begin.CommandList->SetGraphicsConstants(mPipeline, &constant, sizeof(constant));
                    begin.CommandList->DrawIndexed(primitive.IndexCount, 1, 0, 0, 0);
                }
            }
        }
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}

void GBuffer::UI(RenderPassBegin& begin)
{
    if (ImGui::TreeNodeEx("Terrain", ImGuiTreeNodeFlags_Framed)) {
        ImGui::Checkbox("Wireframe", &mWireframe);
        ImGui::TreePop();
    }
}
