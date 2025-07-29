//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 21:16:57
//

#include "FX_DebugRenderer.h"

#include <Graphics/Gfx_ShaderManager.h>
#include <Graphics/Gfx_ResourceManager.h>
#include <Graphics/Gfx_ViewRecycler.h>

#include <glm/gtc/matrix_transform.hpp>

namespace FX
{
    DebugRenderer::Data DebugRenderer::sData;

    float3 GetNormalizedPerpendicular(float3 base)
    {
        if (abs(base.x) > abs(base.y)) {
            float len = sqrt(base.x * base.x + base.y * base.y);
            return float3(base.z, 0.0f, -base.x) / len;
        } else {
            float len = sqrt(base.y * base.y + base.z * base.z);
            return float3(0.0f, base.z, -base.y) / len;
        }
    }

    void DebugRenderer::Initialize()
    {
        // Pipeline
        KGPU::GraphicsPipelineDesc desc;
        desc.CullMode = KGPU::CullMode::kNone;
        desc.DepthEnabled = false;
        desc.LineTopology = true;
        desc.FillMode = KGPU::FillMode::kWireframe;
        desc.RenderTargetFormats.push_back(KGPU::TextureFormat::kR8G8B8A8_UNORM);

        Gfx::ShaderManager::SubscribeGraphics("data/kd/shaders/debug.kds", desc);

        // Resource creation
        KGPU::BufferDesc bufferDesc(sizeof(LineVertex) * MAX_LINES, sizeof(LineVertex), KGPU::BufferUsage::kStaging);

        Gfx::ResourceManager::CreateBuffer(DEBUG_RENDERER_TB_ID, bufferDesc);
        bufferDesc.Usage = KGPU::BufferUsage::kShaderRead;
        Gfx::ResourceManager::CreateBuffer(DEBUG_RENDERER_VB_ID, bufferDesc);
    }

    void DebugRenderer::Render(DebugRendererDesc& desc)
    {
        if (!sData.Lines.empty()) {
            Gfx::Resource& tb = Gfx::ResourceManager::Get(DEBUG_RENDERER_TB_ID);

            KC::Array<LineVertex> vertices;
            for (const Line& line : sData.Lines) {
                vertices.push_back({ line.From, line.Color });
                vertices.push_back({ line.To, line.Color });
            }

            uint uploadSize = std::min(vertices.size() * sizeof(LineVertex), sizeof(LineVertex) * MAX_LINES);
            void* ptr = tb.Buffer->Map();
            memcpy(ptr, vertices.data(), uploadSize);
            tb.Buffer->Unmap();

            desc.CommandList->PushMarker("FX::DebugRenderer::Render");
            CopyBuffer(desc);
            RenderLines(desc);
            desc.CommandList->PopMarker();

            sData.Lines.clear();
        }
    }

    void DebugRenderer::CopyBuffer(DebugRendererDesc& desc)
    {
        Gfx::Resource& tb = Gfx::ResourceManager::Get(DEBUG_RENDERER_TB_ID);
        Gfx::Resource& vb = Gfx::ResourceManager::Get(DEBUG_RENDERER_VB_ID);
    
        KGPU::ScopedMarker(desc.CommandList, "FX::DebugRenderer::CopyBuffer");
        KGPU::BufferBarrier transferStart(tb.Buffer, KGPU::ResourceAccess::kMemoryWrite, KGPU::ResourceAccess::kTransferRead, KGPU::PipelineStage::kAllCommands, KGPU::PipelineStage::kCopy);
        KGPU::BufferBarrier vertexStart(vb.Buffer, KGPU::ResourceAccess::kVertexBufferRead, KGPU::ResourceAccess::kTransferWrite, KGPU::PipelineStage::kAllGraphics, KGPU::PipelineStage::kCopy);

        KGPU::BufferBarrier transferEnd(tb.Buffer, KGPU::ResourceAccess::kTransferRead, KGPU::ResourceAccess::kMemoryWrite, KGPU::PipelineStage::kCopy, KGPU::PipelineStage::kAllCommands);
        KGPU::BufferBarrier vertexEnd(vb.Buffer, KGPU::ResourceAccess::kTransferWrite, KGPU::ResourceAccess::kVertexBufferRead, KGPU::PipelineStage::kCopy, KGPU::PipelineStage::kAllGraphics);

        KGPU::BarrierGroup beginGroup = { {}, { transferStart, vertexStart } };
        KGPU::BarrierGroup endGroup = { {}, { transferEnd, vertexEnd } };

        desc.CommandList->Barrier(beginGroup);
        desc.CommandList->CopyBufferToBufferFull(vb.Buffer, tb.Buffer);
        desc.CommandList->Barrier(endGroup);
    }

    void DebugRenderer::RenderLines(DebugRendererDesc& desc)
    {
        KGPU::ScopedMarker(desc.CommandList, "FX::DebugRenderer::RenderLines");

        Gfx::Resource& vb = Gfx::ResourceManager::Get(DEBUG_RENDERER_VB_ID);
        struct Constants {
            KGPU::float4x4 Proj;
            KGPU::float4x4 View;

            KGPU::BindlessHandle VB;
            KGPU::float3 Pad;
        } constants = {
            desc.Projection,
            desc.View,

            Gfx::ViewRecycler::GetSRV(vb.Buffer)->GetBindlessHandle()
        };

        KGPU::TextureDesc texDesc = desc.RenderTexture->GetDesc();
        KGPU::RenderBegin renderBegin(texDesc.Width, texDesc.Height, { KGPU::RenderAttachment(Gfx::ViewRecycler::GetRTV(desc.RenderTexture), false) }, {});
        KGPU::IGraphicsPipeline* pipeline = Gfx::ShaderManager::GetGraphics("data/kd/shaders/debug.kds");

        desc.CommandList->BeginRendering(renderBegin);
        desc.CommandList->SetRenderSize(texDesc.Width, texDesc.Height);
        desc.CommandList->SetGraphicsPipeline(pipeline);
        desc.CommandList->SetGraphicsConstants(pipeline, &constants, sizeof(constants));
        desc.CommandList->Draw(sData.Lines.size() * 2, 1, 0, 0);
        desc.CommandList->EndRendering();
    }

    void DebugRenderer::DrawLine(float3 from, float3 to, float3 color)
    {
        sData.Lines.push_back(
            { from, to, color }
        );
    }
    
    void DebugRenderer::DrawTriangle(float3 a, float3 b, float3 c, float3 color)
    {
        sData.Lines.push_back(
            { a, b, color }
        );
        sData.Lines.push_back(
            { b, c, color }
        );
        sData.Lines.push_back(
            { c, a, color }
        );
    }
    
    void DebugRenderer::DrawArrow(float3 from, float3 to, float3 color, float size)
    {
        DrawLine(from, to, color);
        
        if (size > 0.0f) {
            float3 dir = to - from;
            float len = glm::length(dir);
            if (len != 0.0f)
                dir = dir * (size / len);
            else
                dir = float3(size, 0, 0);
            float3 perp = size * GetNormalizedPerpendicular(dir);
            DrawLine(to - dir + perp, to, color);
            DrawLine(to - dir - perp, to, color);
        }
    }
    
    void DebugRenderer::DrawUnitBox(glm::mat4 transform, float3 color)
    {
        DrawBox(transform, float3(-1.0f), float3(1.0f), color);
    }
    
    void DebugRenderer::DrawBox(glm::mat4 transform, float3 min, float3 max, float3 color)
    {
        // Transform corners from local space to world space
        float3 v1 = float3(transform * float4(min.x, min.y, min.z, 1.0));
        float3 v2 = float3(transform * float4(min.x, min.y, max.z, 1.0));
        float3 v3 = float3(transform * float4(min.x, max.y, min.z, 1.0));
        float3 v4 = float3(transform * float4(min.x, max.y, max.z, 1.0));
        float3 v5 = float3(transform * float4(max.x, min.y, min.z, 1.0));
        float3 v6 = float3(transform * float4(max.x, min.y, max.z, 1.0));
        float3 v7 = float3(transform * float4(max.x, max.y, min.z, 1.0));
        float3 v8 = float3(transform * float4(max.x, max.y, max.z, 1.0));
    
        // Draw 12 edges of the box
        DrawLine(v1, v2, color); // Edge 1
        DrawLine(v1, v3, color); // Edge 2
        DrawLine(v1, v5, color); // Edge 3
        DrawLine(v2, v4, color); // Edge 4
        DrawLine(v2, v6, color); // Edge 5
        DrawLine(v3, v4, color); // Edge 6
        DrawLine(v3, v7, color); // Edge 7
        DrawLine(v4, v8, color); // Edge 8
        DrawLine(v5, v6, color); // Edge 9
        DrawLine(v5, v7, color); // Edge 10
        DrawLine(v6, v8, color); // Edge 11
        DrawLine(v7, v8, color); // Edge 12
    }
    
    void DebugRenderer::DrawFrustum(glm::mat4 view, glm::mat4 projection, float3 color)
    {
        DrawFrustum(projection * view, color);
    }
    
    void DebugRenderer::DrawFrustum(glm::mat4 projview, float3 color)
    {
        float3 corners[8] = {
            float3(-1.0f,  1.0f, 0.0f),
            float3( 1.0f,  1.0f, 0.0f),
            float3( 1.0f, -1.0f, 0.0f),
            float3(-1.0f, -1.0f, 0.0f),
            float3(-1.0f,  1.0f, 1.0f),
            float3( 1.0f,  1.0f, 1.0f),
            float3( 1.0f, -1.0f, 1.0f),
            float3(-1.0f, -1.0f, 1.0f),
        };
    
        // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
        // Not sure I 100% understand the math here, TODO: study
        for (int i = 0; i < 8; i++) {
            float4 v = float4(corners[i], 1.0);
            float4 h = glm::inverse(projview) * v;
            h.x /= h.w;
            h.y /= h.w;
            h.z /= h.w;
            corners[i] = float3(h);
        }
    
        for (int i = 0; i < 4; i++) {
            DrawLine(corners[i % 4],     corners[(i + 1) % 4],     color);
            DrawLine(corners[i],         corners[i + 4],           color);
            DrawLine(corners[i % 4 + 4], corners[(i + 1) % 4 + 4], color);
        }
    }
    
    void DebugRenderer::DrawFrustumCorners(const glm::mat4& viewToWorld, float3 corners[8], float3 color)
    {
        auto W = [&](const float3& v) {
            return float3(viewToWorld * float4(v, 1.0f));
        };
    
        // Connect edges of the frustum
        DrawLine(W(corners[0]), W(corners[1]), color); // near top
        DrawLine(W(corners[1]), W(corners[2]), color); // near right
        DrawLine(W(corners[2]), W(corners[3]), color); // near bottom
        DrawLine(W(corners[3]), W(corners[0]), color); // near left
    
        DrawLine(W(corners[4]), W(corners[5]), color); // far top
        DrawLine(W(corners[5]), W(corners[6]), color); // far right
        DrawLine(W(corners[6]), W(corners[7]), color); // far bottom
        DrawLine(W(corners[7]), W(corners[4]), color); // far left
    
        // Connect near to far
        for (int i = 0; i < 4; ++i)
            DrawLine(W(corners[i]), W(corners[i + 4]), color);
    }
    
    void DebugRenderer::DrawCoordinateSystem(glm::mat4 transform, float size)
    {
        float3 translation = float3(transform[0][3], transform[1][3], transform[2][3]);
        DrawArrow(translation, transform * float4(size, 0, 0, 1.0f), float3(1.0f, 0.0f, 0.0f), 0.1f * size);
        DrawArrow(translation, transform * float4(0, size, 0, 1.0f), float3(0.0f, 1.0f, 0.0f), 0.1f * size);
        DrawArrow(translation, transform * float4(0, 0, size, 1.0f), float3(0.0f, 0.0f, 1.0f), 0.1f * size);
    }
    
    void DebugRenderer::DrawSphere(float3 center, float radius, float3 color, int level)
    {
        glm::mat4 matrix = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), float3(radius));
        float3 xAxis = float3(1.0f, 0.0f, 0.0f);
        float3 yAxis = float3(0.0f, 1.0f, 0.0f);
        float3 zAxis = float3(0.0f, 0.0f, 1.0f);
    
        DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis,  zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis,  zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis,  zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis,  zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis, -zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis, -zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis, -zAxis, level);
        DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis, -zAxis, level);
    }
    
    void DebugRenderer::DrawRing(float3 center, float3 normal, float radius, float3 color, int level)
    {
        // Ensure a minimum of 3 segments to form a polygon
        int numSegments = glm::max(level, 3);
    
        float3 tangent;
        if (glm::abs(normal.y) > 0.99f) {
            tangent = float3(1.0f, 0.0f, 0.0f);
        } else {
            tangent = glm::normalize(glm::cross(normal, float3(0.0f, 1.0f, 0.0f)));
        }
        float3 bitangent = glm::normalize(glm::cross(normal, tangent));
    
        float angleStep = glm::two_pi<float>() / numSegments;
        float3 prevPoint = center + radius * tangent;
    
        for (int i = 1; i <= numSegments; ++i) {
            float angle = i * angleStep;
            float3 nextPoint = center + radius * (cos(angle) * tangent + sin(angle) * bitangent);
        
            DebugRenderer::DrawLine(prevPoint, nextPoint, color);
        
            prevPoint = nextPoint;
        }
    }
    
    void DebugRenderer::DrawRings(float3 center, float radius, float3 color, int level)
    {
        DrawRing(center, float3(1.0f, 0.0f, 0.0f), radius, color, level);
        DrawRing(center, float3(0.0f, 1.0f, 0.0f), radius, color, level);
        DrawRing(center, float3(0.0f, 0.0f, 1.0f), radius, color, level);
    }
    
    void DebugRenderer::DrawQuad(glm::mat4 transform, float3 corners[4], float3 color)
    {
        // Transform the 4 corners to world space
        float3 v0 = float3(transform * float4(corners[0], 1.0f));
        float3 v1 = float3(transform * float4(corners[1], 1.0f));
        float3 v2 = float3(transform * float4(corners[2], 1.0f));
        float3 v3 = float3(transform * float4(corners[3], 1.0f));
    
        // Draw the quad edges (assumes corners ordered consistently)
        DrawLine(v0, v1, color);
        DrawLine(v1, v2, color);
        DrawLine(v2, v3, color);
        DrawLine(v3, v0, color);
    }
    
    void DebugRenderer::DrawCone(glm::mat4 transform, float3 position, float size, float3 forward, float angle, float3 color)
    {
        float3 pos = float3(transform * float4(position, 1.0f));
        const int ringSegments = 16;
        
        // Normalize forward vector to ensure correct calculations
        float3 normalizedForward = glm::normalize(forward);
        
        // Compute cone base center
        float3 endCenter = pos + normalizedForward * size;
        
        // Get cone base radius
        float angleRad = angle / 2.0f;
        float radius = size * tan(angleRad);
        
        // Generate orthonormal basis for ring
        float3 up = fabs(normalizedForward.y) < 0.99f ? float3(0,1,0) : float3(1,0,0);
        float3 right = glm::normalize(glm::cross(normalizedForward, up));
        up = glm::normalize(glm::cross(right, normalizedForward)); // Complete the orthonormal basis
        
        // Draw base ring
        DrawRing(endCenter, normalizedForward, radius, color);
        
        // Draw cone edges (connect apex to points on the base circle)
        for (int i = 0; i < ringSegments; i += ringSegments / 4) {
            float theta = (float)i / (float)ringSegments * glm::two_pi<float>();
            float x = cos(theta) * radius;
            float y = sin(theta) * radius;
            float3 ringPoint = endCenter + right * x + up * y; // Use 'up' instead of 'forward'
            DrawLine(pos, ringPoint, color);
        }
        
        // Optional: draw forward direction line (axis of the cone)
        DrawArrow(pos, endCenter, color);
    }
    
    void DebugRenderer::DrawWireUnitSphereRecursive(glm::mat4 matrix, float3 inColor, float3 inDir1, float3 inDir2, float3 inDir3, int inLevel)
    {
        if (inLevel == 0) {
            float3 d1 = matrix * float4(inDir1, 1.0f);
            float3 d2 = matrix * float4(inDir2, 1.0f);
            float3 d3 = matrix * float4(inDir3, 1.0f);
        
            DrawLine(d1, d2, inColor);
            DrawLine(d2, d3, inColor);
            DrawLine(d3, d1, inColor);
        } else {
            float3 center1 = glm::normalize(inDir1 + inDir2);
            float3 center2 = glm::normalize(inDir2 + inDir3);
            float3 center3 = glm::normalize(inDir3 + inDir1);
        
            DrawWireUnitSphereRecursive(matrix, inColor, inDir1, center1, center3, inLevel - 1);
            DrawWireUnitSphereRecursive(matrix, inColor, center1, center2, center3, inLevel - 1);
            DrawWireUnitSphereRecursive(matrix, inColor, center1, inDir2, center2, inLevel - 1);
            DrawWireUnitSphereRecursive(matrix, inColor, center3, center2, inDir3, inLevel - 1);
        }
    }
}
