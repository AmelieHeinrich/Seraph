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
        glm::vec3 From;
        glm::vec3 To;
        glm::vec3 Color;
    };

    Debug(IRHIDevice* device, uint width, uint height);
    ~Debug();

    void Render(RenderPassBegin& begin) override;

    static void DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1.0f));
    static void DrawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color = glm::vec3(1.0f));
    static void DrawArrow(glm::vec3 from, glm::vec3 to, glm::vec3 color = glm::vec3(1.0f), float size = 0.1f);
    static void DrawUnitBox(glm::mat4 transform, glm::vec3 color = glm::vec3(1.0f));
    static void DrawBox(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color = glm::vec3(1.0f));
    static void DrawFrustum(glm::mat4 view, glm::mat4 projection, glm::vec3 color = glm::vec3(1.0f));
    static void DrawFrustum(glm::mat4 projview, glm::vec3 color = glm::vec3(1.0f));
    static void DrawCoordinateSystem(glm::mat4 transform, float size);
    static void DrawSphere(glm::vec3 center, float radius, glm::vec3 color = glm::vec3(1.0f), int level = 3);
    static void DrawRing(glm::vec3 center, glm::vec3 normal, float radius, glm::vec3 color = glm::vec3(1.0f), int level = 32);
    static void DrawRings(glm::vec3 center, float radius, glm::vec3 color = glm::vec3(1.0f), int level = 32);

private:
    static constexpr uint MAX_LINES = 5192 * 16;

    void CopyToVB(RenderPassBegin& begin);
    void RenderLines(RenderPassBegin& begin);

    static void DrawWireUnitSphereRecursive(glm::mat4 matrix, glm::vec3 inColor, glm::vec3 inDir1, glm::vec3 inDir2, glm::vec3 inDir3, int inLevel);

    struct LineVertex
    {
        glm::vec3 Position;
        float Pad;
    
        glm::vec3 Color;
        float Pad1;
    };

    static struct Data
    {
        Array<Line> Lines;
        IRHIGraphicsPipeline* Pipeline;
        StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> TransferBuffer;
        StaticArray<IRHIBuffer*, FRAMES_IN_FLIGHT> VertexBuffer;
    } sData;

    uint64 mLineCount = 0;
};
