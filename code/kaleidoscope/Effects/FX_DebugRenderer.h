//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-23 21:15:55
//

#pragma once

#include <Graphics/Gfx_Manager.h>

namespace FX
{
    struct DebugRendererDesc
    {
        KGPU::ICommandList* CommandList;
        KGPU::ITexture* RenderTexture;

        KGPU::float4x4 Projection;
        KGPU::float4x4 View;
    };

    using namespace KGPU;

    class DebugRenderer
    {
    public:
        static void Initialize();

        static void Render(DebugRendererDesc& desc);

        static void DrawLine(float3 from, float3 to, float3 color = float3(1.0f));
        static void DrawTriangle(float3 a, float3 b, float3 c, float3 color = float3(1.0f));
        static void DrawArrow(float3 from, float3 to, float3 color = float3(1.0f), float size = 0.1f);
        static void DrawUnitBox(glm::mat4 transform, float3 color = float3(1.0f));
        static void DrawBox(glm::mat4 transform, float3 min, float3 max, float3 color = float3(1.0f));
        static void DrawFrustum(glm::mat4 view, glm::mat4 projection, float3 color = float3(1.0f));
        static void DrawFrustum(glm::mat4 projview, float3 color = float3(1.0f));
        static void DrawFrustumCorners(const glm::mat4& viewToWorld, float3 corners[8], float3 color = float3(1.0f));
        static void DrawCoordinateSystem(glm::mat4 transform, float size);
        static void DrawSphere(float3 center, float radius, float3 color = float3(1.0f), int level = 3);
        static void DrawRing(float3 center, float3 normal, float radius, float3 color = float3(1.0f), int level = 32);
        static void DrawRings(float3 center, float radius, float3 color = float3(1.0f), int level = 32);
        static void DrawQuad(glm::mat4 transform, float3 corners[4], float3 color = float3(1.0f));
        static void DrawCone(glm::mat4 transform, float3 position, float size, float3 forward, float angle, float3 color = float3(1.0f));
        static void DrawWireUnitSphereRecursive(glm::mat4 matrix, float3 inColor, float3 inDir1, float3 inDir2, float3 inDir3, int inLevel);

    private:
        static void CopyBuffer(DebugRendererDesc& desc);
        static void RenderLines(DebugRendererDesc& desc);

        static constexpr uint MAX_LINES = 16384 * 16;
        static constexpr const char* DEBUG_RENDERER_TB_ID = "DebugRenderer/TransferBuffer";
        static constexpr const char* DEBUG_RENDERER_VB_ID = "DebugRenderer/VertexBuffer";

        struct Line
        {
            KGPU::float3 From;
            KGPU::float3 To;
            KGPU::float3 Color;
        };

        struct LineVertex
        {
            KGPU::float3 Position;
            KGPU::float3 Color;
        };

        static struct Data {
            KC::Array<Line> Lines;
        } sData;
    };
}
