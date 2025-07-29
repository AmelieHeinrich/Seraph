//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-17 21:38:19
//

#include "ToolImGui_Renderer.h"

#include <KDShader/KDS_ShaderFile.h>
#include <Graphics/Gfx_TempResourceStorage.h>
#include <Graphics/Gfx_CommandListRecycler.h>

#include <imgui.h>

namespace ToolImGui
{
    Renderer::Renderer(KGPU::IDevice* device)
        : mDevice(device)
    {
        CreatePipeline(device);

        ImGuiIO& io = ImGui::GetIO();
        io.BackendRendererName = "imgui_impl_kd";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; 
        io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    }

    Renderer::~Renderer()
    {
        for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
            if (tex->RefCount == 1) {
                ImGuiFontTexture* texture = (ImGuiFontTexture*)tex->BackendUserData;

                KC_DELETE(texture->Tex);
                KC_DELETE(texture->View);
                KC_DELETE(texture);

                tex->SetTexID(ImTextureID_Invalid);
                tex->SetStatus(ImTextureStatus_Destroyed);
                tex->BackendUserData = nullptr;
            }
        }
        for (int i = 0; i < KGPU::FRAMES_IN_FLIGHT; i++) {
            FrameResource& resource = mFrameResources[i];

            KC_DELETE(resource.mVertexBufferView);
            KC_DELETE(resource.mVertexBuffer);
            KC_DELETE(resource.mIndexBuffer);
        }
        KC_DELETE(mFontTextureView);
        KC_DELETE(mFontTexture);
        KC_DELETE(mSampler);
        KC_DELETE(mPipeline);
    }

    void Renderer::Render(ImDrawData* data, KGPU::ICommandList* commandList, int frameIndex)
    {
        commandList->PushMarker("ToolImGui::Renderer::Render");
        UpdateTexture(mDevice, data, commandList);

        FrameResource& resource = mFrameResources[frameIndex];

        // Recreate buffers
        if (resource.mVertexBuffer == nullptr || resource.mVertexBufferSize < data->TotalVtxCount) {
            KC_DELETE(resource.mVertexBufferView);
            KC_DELETE(resource.mVertexBuffer);

            resource.mVertexBufferSize = data->TotalVtxCount + 5000;
            resource.mVertexBuffer = mDevice->CreateBuffer(KGPU::BufferDesc(resource.mVertexBufferSize * sizeof(ImDrawVert), sizeof(ImDrawVert), KGPU::BufferUsage::kStaging | KGPU::BufferUsage::kShaderRead));
            resource.mVertexBufferView = mDevice->CreateBufferView(KGPU::BufferViewDesc(resource.mVertexBuffer, KGPU::BufferViewType::kStructured));
        }
        if (resource.mIndexBuffer == nullptr || resource.mIndexBufferSize < data->TotalIdxCount) {
            KC_DELETE(resource.mIndexBuffer);

            resource.mIndexBufferSize = data->TotalIdxCount + 5000;
            resource.mIndexBuffer = mDevice->CreateBuffer(KGPU::BufferDesc(resource.mIndexBufferSize * sizeof(ImDrawIdx), sizeof(ImDrawIdx), KGPU::BufferUsage::kStaging | KGPU::BufferUsage::kIndex));
        }

        // Copy data
        void* vtx_resource = resource.mVertexBuffer->Map();
        void* idx_resource = resource.mIndexBuffer->Map();

        ImDrawVert* vtx_dst = (ImDrawVert*)vtx_resource;
        ImDrawIdx* idx_dst = (ImDrawIdx*)idx_resource;

        for (int n = 0; n < data->CmdListsCount; n++) {
            const ImDrawList* drawList = data->CmdLists[n];
            memcpy(vtx_dst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += drawList->VtxBuffer.Size;
            idx_dst += drawList->IdxBuffer.Size;
        }

        resource.mVertexBuffer->Unmap();
        resource.mIndexBuffer->Unmap();

        // Setup matrix
        float L = data->DisplayPos.x;
        float R = data->DisplayPos.x + data->DisplaySize.x;
        float T = data->DisplayPos.y;
        float B = data->DisplayPos.y + data->DisplaySize.y;
        KGPU::float4x4 mvp = {
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        };

        // Setup render state
        struct PushConstants {
            KGPU::float4x4 Proj;

            KGPU::BindlessHandle VertexIndex;
            KGPU::BindlessHandle SamplerIndex;
            KGPU::BindlessHandle TextureIndex;
            int VertexOffset;
        } push = {
            mvp,

            resource.mVertexBufferView->GetBindlessHandle(),
            mSampler->GetBindlessHandle()
        };

        commandList->SetViewport(data->DisplaySize.x * data->FramebufferScale.x, data->DisplaySize.y * data->FramebufferScale.y, 0, 0);
        commandList->SetIndexBuffer(resource.mIndexBuffer);
        commandList->SetGraphicsPipeline(mPipeline);

        // Render command list
        int global_vtx_offset = 0;
        int global_idx_offset = 0;
        ImVec2 clip_off = data->DisplayPos;
        ImVec2 clip_scale = data->FramebufferScale;
        for (int n = 0; n < data->CmdListsCount; n++)
        {
            const ImDrawList* drawList = data->CmdLists[n];
            for (int cmd_i = 0; cmd_i < drawList->CmdBuffer.Size; cmd_i++)
            {
                const ImDrawCmd* pcmd = &drawList->CmdBuffer[cmd_i];
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x, (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x, (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);
            
                push.TextureIndex = (KGPU::BindlessHandle)pcmd->GetTexID();
                push.VertexOffset = pcmd->VtxOffset + global_vtx_offset;

                // Apply scissor/clipping rectangle, bind, draw
                if (mDevice->GetBackend() == KGPU::Backend::kD3D12) {
                    commandList->SetScissor(clip_min.x, clip_min.y, clip_max.x, clip_max.y);
                } else {
                    commandList->SetScissor(clip_min.x, clip_min.y, clip_max.x - clip_min.x, clip_max.y - clip_min.y);
                }
                commandList->SetGraphicsConstants(mPipeline, &push, sizeof(push));
                commandList->DrawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset + global_idx_offset, 0, 0);
            }
            global_idx_offset += drawList->IdxBuffer.Size;
            global_vtx_offset += drawList->VtxBuffer.Size;
        }
        commandList->PopMarker();
    }

    void Renderer::UpdateTexture(KGPU::IDevice* device, ImDrawData* data, KGPU::ICommandList* commandList)
    {
        if (data->Textures != nullptr) {
            for (ImTextureData* tex : *data->Textures) {
                if (tex->Status != ImTextureStatus_OK) {
                    // Update!

                    if (tex->Status == ImTextureStatus_WantDestroy && tex->UnusedFrames > KGPU::FRAMES_IN_FLIGHT) {
                        ImGuiFontTexture* texture = (ImGuiFontTexture*)tex->BackendUserData;

                        KC_DELETE(texture->Tex);
                        KC_DELETE(texture->View);
                        KC_DELETE(texture);

                        tex->SetTexID(ImTextureID_Invalid);
                        tex->SetStatus(ImTextureStatus_Destroyed);
                        tex->BackendUserData = nullptr;
                    }
                    if (tex->Status == ImTextureStatus_WantCreate) {
                        KGPU::TextureDesc fontDesc;
                        fontDesc.Width = tex->Width;
                        fontDesc.Height = tex->Height;
                        fontDesc.Format = KGPU::TextureFormat::kR8G8B8A8_UNORM;
                        fontDesc.Usage = KGPU::TextureUsage::kShaderResource;

                        ImGuiFontTexture* texture = KC_NEW(ImGuiFontTexture);
                        texture->Tex = mDevice->CreateTexture(fontDesc);
                        texture->Tex->SetName("ImGui Texture");
                        texture->View = mDevice->CreateTextureView(KGPU::TextureViewDesc(texture->Tex, KGPU::TextureViewType::kShaderRead));
                        texture->UploadedAtLeastOnce = false;
                        
                        tex->SetTexID(texture->View->GetBindlessHandle().Index);
                        tex->BackendUserData = texture;
                        tex->SetStatus(ImTextureStatus_WantUpdates);
                    }

                    if (tex->Status == ImTextureStatus_WantUpdates) {
                        ImGuiFontTexture* texture = (ImGuiFontTexture*)tex->BackendUserData;

                        KGPU::BufferDesc stagingDesc = {};
                        stagingDesc.Size = tex->GetSizeInBytes();
                        stagingDesc.Usage = KGPU::BufferUsage::kStaging;
                    
                        KGPU::IBuffer* stagingBuffer = Gfx::TempResourceStorage::CreateBuffer(stagingDesc);
                        stagingBuffer->SetName("Staging Buffer");
                        void* mappedVoid = stagingBuffer->Map();
                        memcpy(mappedVoid, tex->GetPixels(), tex->GetSizeInBytes());
                        stagingBuffer->Unmap();

                        KGPU::TextureBarrier dstBarrier(texture->Tex);
                        dstBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
                        dstBarrier.DestStage = KGPU::PipelineStage::kCopy;
                        dstBarrier.SourceAccess = texture->UploadedAtLeastOnce ? KGPU::ResourceAccess::kShaderRead : KGPU::ResourceAccess::kNone;
                        dstBarrier.DestAccess = KGPU::ResourceAccess::kTransferWrite;
                        dstBarrier.NewLayout = KGPU::ResourceLayout::kTransferDst;
                        dstBarrier.BaseMipLevel = 0;
                        dstBarrier.LevelCount = 1;

                        KGPU::BufferBarrier stagingBarrier(stagingBuffer);
                        stagingBarrier.SourceStage = KGPU::PipelineStage::kAllCommands;
                        stagingBarrier.DestStage = KGPU::PipelineStage::kCopy;
                        stagingBarrier.SourceAccess = KGPU::ResourceAccess::kNone;
                        stagingBarrier.DestAccess = KGPU::ResourceAccess::kTransferRead;

                        KGPU::BarrierGroup firstGroup = {};
                        firstGroup.BufferBarriers = { stagingBarrier };
                        firstGroup.TextureBarriers = { dstBarrier };

                        KGPU::TextureBarrier dstBarrierAfter(texture->Tex);
                        dstBarrierAfter.SourceStage = KGPU::PipelineStage::kCopy;
                        dstBarrierAfter.DestStage = KGPU::PipelineStage::kAllGraphics;
                        dstBarrierAfter.SourceAccess = KGPU::ResourceAccess::kTransferWrite;
                        dstBarrierAfter.DestAccess = KGPU::ResourceAccess::kShaderRead;
                        dstBarrierAfter.NewLayout = KGPU::ResourceLayout::kReadOnly;
                        dstBarrierAfter.BaseMipLevel = 0;
                        dstBarrierAfter.LevelCount = 1;

                        KGPU::ICommandList* list = Gfx::CommandListRecycler::RequestCommandList();
                        list->Barrier(firstGroup);
                        list->CopyBufferToTexture(texture->Tex, stagingBuffer, false);
                        list->Barrier(dstBarrierAfter);
                        Gfx::CommandListRecycler::FlushCommandLists();

                        tex->SetStatus(ImTextureStatus_OK);
                        texture->UploadedAtLeastOnce = true;
                    }
                }
            }
        }
    }

    void Renderer::CreatePipeline(KGPU::IDevice* device)
    {
        // Pipeline
        KDS::ICompiler* compiler = KDS::ICompiler::Create(device->GetTargetBytecode());
        KDS::IReflectionEngine* reflectionEngine = KDS::IReflectionEngine::Create(device->GetTargetBytecode());

        KDS::ShaderFile file(compiler, reflectionEngine, "data/kd/shaders/imgui.kds");

        KC_DELETE(compiler);
        KC_DELETE(reflectionEngine);

        KGPU::GraphicsPipelineDesc desc;
        desc.CounterClockwise = false;
        desc.CullMode = KGPU::CullMode::kNone;
        desc.DepthClampEnabled = false;
        desc.DepthEnabled = false;
        desc.DepthWrite = false;
        desc.EnableBlend = true;
        desc.FillMode = KGPU::FillMode::kSolid;
        desc.LineTopology = false;
        desc.ReflectInputLayout = false;
        desc.RenderTargetFormats.push_back(device->GetSurfaceFormat());
        desc.Modules[KGPU::ShaderStage::kVertex] = file.Get(KGPU::ShaderStage::kVertex);
        desc.Modules[KGPU::ShaderStage::kPixel] = file.Get(KGPU::ShaderStage::kPixel);
        desc.PushConstantSize = file.GetReflection().PushConstantSize;

        mPipeline = device->CreateGraphicsPipeline(desc);

        // Sampler
        mSampler = device->CreateSampler(KGPU::SamplerDesc(KGPU::SamplerAddress::kClamp, KGPU::SamplerFilter::kLinear, false));
    }
}
