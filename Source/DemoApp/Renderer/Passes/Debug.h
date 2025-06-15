//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-09 16:23:21
//

#pragma once

#include <DemoApp/Renderer/RenderPass.h>

class Debug : public RenderPass
{
public:
    struct Line
    {
        float3 From;
        float3 To;
        float3 Color;
    };

    Debug(IRHIDevice* device, uint width, uint height);
    ~Debug();

    void Render(RenderPassBegin& begin) override;
    void UI(RenderPassBegin& begin) override;

    static void DrawLine(float3 from, float3 to, float3 color = float3(1.0f));
    static void DrawTriangle(float3 a, float3 b, float3 c, float3 color = float3(1.0f));
    static void DrawArrow(float3 from, float3 to, float3 color = float3(1.0f), float size = 0.1f);
    static void DrawUnitBox(glm::mat4 transform, float3 color = float3(1.0f));
    static void DrawBox(glm::mat4 transform, float3 min, float3 max, float3 color = float3(1.0f));
    static void DrawFrustum(glm::mat4 view, glm::mat4 projection, float3 color = float3(1.0f));
    static void DrawFrustum(glm::mat4 projview, float3 color = float3(1.0f));
    static void DrawFrustumCorners(const glm::mat4& viewToWorld, const StaticArray<float3, 8>& corners, float3 color = float3(1.0f));
    static void DrawCoordinateSystem(glm::mat4 transform, float size);
    static void DrawSphere(float3 center, float radius, float3 color = float3(1.0f), int level = 3);
    static void DrawRing(float3 center, float3 normal, float radius, float3 color = float3(1.0f), int level = 32);
    static void DrawRings(float3 center, float radius, float3 color = float3(1.0f), int level = 32);
    static void DrawQuad(glm::mat4 transform, const StaticArray<float3, 4>& corners, float3 color = float3(1.0f));
    static void DrawCone(glm::mat4 transform, float3 position, float size, float3 forward, float angle, float3 color = float3(1.0f));

private:
    static constexpr uint MAX_LINES = 16384 * 16;

    void CopyToVB(RenderPassBegin& begin);
    void RenderLines(RenderPassBegin& begin);

    static void DrawWireUnitSphereRecursive(glm::mat4 matrix, float3 inColor, float3 inDir1, float3 inDir2, float3 inDir3, int inLevel);

    struct LineVertex
    {
        float3 Position;
        float Pad;
    
        float3 Color;
        float Pad1;
    };

    static struct Data
    {
        Array<Line> Lines;
        IRHIGraphicsPipeline* Pipeline;
        IRHIGraphicsPipeline* NoDepthPipeline;
        StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> TransferBuffer;
        StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> VertexBuffer;

        bool UseDepth = false;
    } sData;

    uint64 mLineCount = 0;
};
