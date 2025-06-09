//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-09 16:26:58
//

#include "Debug.h"
#include "Tonemapping.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

Debug::Data Debug::sData;

glm::vec3 GetNormalizedPerpendicular(glm::vec3 base)
{
    if (abs(base.x) > abs(base.y)) {
        float len = sqrt(base.x * base.x + base.y * base.y);
        return glm::vec3(base.z, 0.0f, -base.x) / len;
    } else {
        float len = sqrt(base.y * base.y + base.z * base.z);
        return glm::vec3(0.0f, base.z, -base.y) / len;
    }
}

Debug::Debug(IRHIDevice* device, uint width, uint height)
    : RenderPass(device, width, height)
{
    CompiledShader shader = ShaderCompiler::Compile("Debug", { "VSMain", "FSMain" });

    RHIGraphicsPipelineDesc desc = {};
    desc.Bytecode[ShaderStage::kVertex] = shader.Entries["VSMain"];
    desc.Bytecode[ShaderStage::kFragment] = shader.Entries["FSMain"];
    desc.LineTopology = true;
    desc.CullMode = RHICullMode::kNone;
    desc.RenderTargetFormats.push_back(RHITextureFormat::kR8G8B8A8_UNORM);
    desc.PushConstantSize = sizeof(glm::mat4) * 2;

    sData.Pipeline = device->CreateGraphicsPipeline(desc);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        sData.TransferBuffer[i] = mParentDevice->CreateBuffer(RHIBufferDesc(sizeof(LineVertex) * MAX_LINES, sizeof(LineVertex), RHIBufferUsage::kStaging));
        sData.VertexBuffer[i] = mParentDevice->CreateBuffer(RHIBufferDesc(sizeof(LineVertex) * MAX_LINES, sizeof(LineVertex), RHIBufferUsage::kVertex));
    }
}

Debug::~Debug()
{
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        delete sData.VertexBuffer[i];
        delete sData.TransferBuffer[i];
    }
    delete sData.Pipeline;
}

void Debug::Render(RenderPassBegin& begin)
{
    if (!sData.Lines.empty()) {
        Array<LineVertex> vertices;
        for (const Line& line : sData.Lines) {
            vertices.push_back({ line.From, line.Color });
            vertices.push_back({ line.To, line.Color });
        }

        void* ptr = sData.TransferBuffer[begin.FrameIndex]->Map();
        memcpy(ptr, vertices.data(), vertices.size() * sizeof(LineVertex));
        sData.TransferBuffer[begin.FrameIndex]->Unmap();
  
        begin.CommandList->PushMarker("Debug");
        CopyToVB(begin);
        RenderLines(begin);
        begin.CommandList->PopMarker();

        sData.Lines.clear();
    }
}

void Debug::CopyToVB(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Copy to Vertex Buffer");
    {
        RHIBufferBarrier transferStart(sData.TransferBuffer[begin.FrameIndex], RHIResourceAccess::kMemoryWrite, RHIResourceAccess::kTransferRead, RHIPipelineStage::kAllCommands, RHIPipelineStage::kCopy);
        RHIBufferBarrier vertexStart(sData.VertexBuffer[begin.FrameIndex], RHIResourceAccess::kVertexBufferRead, RHIResourceAccess::kTransferWrite, RHIPipelineStage::kAllGraphics, RHIPipelineStage::kCopy);

        RHIBufferBarrier transferEnd(sData.TransferBuffer[begin.FrameIndex], RHIResourceAccess::kTransferRead, RHIResourceAccess::kMemoryWrite, RHIPipelineStage::kCopy, RHIPipelineStage::kAllCommands);
        RHIBufferBarrier vertexEnd(sData.VertexBuffer[begin.FrameIndex], RHIResourceAccess::kTransferWrite, RHIResourceAccess::kVertexBufferRead, RHIPipelineStage::kCopy, RHIPipelineStage::kAllGraphics);
        
        RHIBarrierGroup beginGroup = { {}, { transferStart, vertexStart } };
        RHIBarrierGroup endGroup = { {}, { transferEnd, vertexEnd } };

        begin.CommandList->BarrierGroup(beginGroup);
        begin.CommandList->CopyBufferToBufferFull(sData.VertexBuffer[begin.FrameIndex], sData.TransferBuffer[begin.FrameIndex]);
        begin.CommandList->BarrierGroup(endGroup);
    }
    begin.CommandList->PopMarker();
}

void Debug::RenderLines(RenderPassBegin& begin)
{
    begin.CommandList->PushMarker("Render Debug Lines");
    {
        glm::mat4 constants[] = {
            begin.Projection,
            begin.View
        };
        RendererResource& ldr = RendererResourceManager::Import(TONEMAPPING_LDR_ID, begin.CommandList, RendererImportType::kColorWrite);
        RHIRenderBegin renderBegin(mWidth, mHeight, { RHIRenderAttachment(RendererViewRecycler::GetRTV(ldr.Texture), false) }, {});

        begin.CommandList->BeginRendering(renderBegin);
        begin.CommandList->SetViewport(mWidth, mHeight, 0, 0);
        begin.CommandList->SetGraphicsPipeline(sData.Pipeline);
        begin.CommandList->SetVertexBuffer(sData.VertexBuffer[begin.FrameIndex]);
        begin.CommandList->SetGraphicsConstants(sData.Pipeline, constants, sizeof(constants));
        begin.CommandList->Draw(sData.Lines.size() * 2, 1, 0, 0);
        begin.CommandList->EndRendering();
    }
    begin.CommandList->PopMarker();
}

void Debug::DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color)
{
    sData.Lines.push_back(
        { from, to, color }
    );
}

void Debug::DrawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color)
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

void Debug::DrawArrow(glm::vec3 from, glm::vec3 to, glm::vec3 color, float size)
{
    DrawLine(from, to, color);
    
    if (size > 0.0f) {
        glm::vec3 dir = to - from;
        float len = glm::length(dir);
        if (len != 0.0f)
            dir = dir * (size / len);
        else
            dir = glm::vec3(size, 0, 0);
        glm::vec3 perp = size * GetNormalizedPerpendicular(dir);
        DrawLine(to - dir + perp, to, color);
        DrawLine(to - dir - perp, to, color);
    }
}

void Debug::DrawUnitBox(glm::mat4 transform, glm::vec3 color)
{
    DrawBox(transform, glm::vec3(-1.0f), glm::vec3(1.0f), color);
}

void Debug::DrawBox(glm::mat4 transform, glm::vec3 min, glm::vec3 max, glm::vec3 color)
{
    // Transform corners from local space to world space
    glm::vec3 v1 = glm::vec3(transform * glm::vec4(min.x, min.y, min.z, 1.0));
    glm::vec3 v2 = glm::vec3(transform * glm::vec4(min.x, min.y, max.z, 1.0));
    glm::vec3 v3 = glm::vec3(transform * glm::vec4(min.x, max.y, min.z, 1.0));
    glm::vec3 v4 = glm::vec3(transform * glm::vec4(min.x, max.y, max.z, 1.0));
    glm::vec3 v5 = glm::vec3(transform * glm::vec4(max.x, min.y, min.z, 1.0));
    glm::vec3 v6 = glm::vec3(transform * glm::vec4(max.x, min.y, max.z, 1.0));
    glm::vec3 v7 = glm::vec3(transform * glm::vec4(max.x, max.y, min.z, 1.0));
    glm::vec3 v8 = glm::vec3(transform * glm::vec4(max.x, max.y, max.z, 1.0));

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

void Debug::DrawFrustum(glm::mat4 view, glm::mat4 projection, glm::vec3 color)
{
    DrawFrustum(projection * view, color);
}

void Debug::DrawFrustum(glm::mat4 projview, glm::vec3 color)
{
    glm::vec3 corners[8] = {
        glm::vec3(-1.0f,  1.0f, 0.0f),
        glm::vec3( 1.0f,  1.0f, 0.0f),
        glm::vec3( 1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f, -1.0f, 0.0f),
        glm::vec3(-1.0f,  1.0f, 1.0f),
        glm::vec3( 1.0f,  1.0f, 1.0f),
        glm::vec3( 1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, -1.0f, 1.0f),
    };

    // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
    // Not sure I 100% understand the math here, TODO: study
    for (int i = 0; i < 8; i++) {
        glm::vec4 v = glm::vec4(corners[i], 1.0);
        glm::vec4 h = glm::inverse(projview) * v;
        h.x /= h.w;
        h.y /= h.w;
        h.z /= h.w;
        corners[i] = glm::vec3(h);
    }

    for (int i = 0; i < 4; i++) {
        DrawLine(corners[i % 4],     corners[(i + 1) % 4],     color);
        DrawLine(corners[i],         corners[i + 4],           color);
        DrawLine(corners[i % 4 + 4], corners[(i + 1) % 4 + 4], color);
    }
}

void Debug::DrawCoordinateSystem(glm::mat4 transform, float size)
{
    glm::vec3 translation = glm::vec3(transform[0][3], transform[1][3], transform[2][3]);
    DrawArrow(translation, transform * glm::vec4(size, 0, 0, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.1f * size);
	DrawArrow(translation, transform * glm::vec4(0, size, 0, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.1f * size);
	DrawArrow(translation, transform * glm::vec4(0, 0, size, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.1f * size);
}

void Debug::DrawSphere(glm::vec3 center, float radius, glm::vec3 color, int level)
{
    glm::mat4 matrix = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    glm::vec3 xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

    DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis,  zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis,  yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis,  yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color,  xAxis, -yAxis, -zAxis, level);
	DrawWireUnitSphereRecursive(matrix, color, -xAxis, -yAxis, -zAxis, level);
}

void Debug::DrawRing(glm::vec3 center, glm::vec3 normal, float radius, glm::vec3 color, int level)
{
    // Ensure a minimum of 3 segments to form a polygon
    int numSegments = glm::max(level, 3);

    glm::vec3 tangent;
    if (glm::abs(normal.y) > 0.99f) {
        tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    } else {
        tangent = glm::normalize(glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));

    float angleStep = glm::two_pi<float>() / numSegments;
    glm::vec3 prevPoint = center + radius * tangent;

    for (int i = 1; i <= numSegments; ++i) {
        float angle = i * angleStep;
        glm::vec3 nextPoint = center + radius * (cos(angle) * tangent + sin(angle) * bitangent);

        Debug::DrawLine(prevPoint, nextPoint, color);

        prevPoint = nextPoint;
    }
}

void Debug::DrawRings(glm::vec3 center, float radius, glm::vec3 color, int level)
{
    DrawRing(center, glm::vec3(1.0f, 0.0f, 0.0f), radius, color, level);
    DrawRing(center, glm::vec3(0.0f, 1.0f, 0.0f), radius, color, level);
    DrawRing(center, glm::vec3(0.0f, 0.0f, 1.0f), radius, color, level);
}

void Debug::DrawWireUnitSphereRecursive(glm::mat4 matrix, glm::vec3 inColor, glm::vec3 inDir1, glm::vec3 inDir2, glm::vec3 inDir3, int inLevel)
{
    if (inLevel == 0) {
		glm::vec3 d1 = matrix * glm::vec4(inDir1, 1.0f);
		glm::vec3 d2 = matrix * glm::vec4(inDir2, 1.0f);
		glm::vec3 d3 = matrix * glm::vec4(inDir3, 1.0f);

		DrawLine(d1, d2, inColor);
		DrawLine(d2, d3, inColor);
		DrawLine(d3, d1, inColor);
	} else {
		glm::vec3 center1 = glm::normalize(inDir1 + inDir2);
		glm::vec3 center2 = glm::normalize(inDir2 + inDir3);
		glm::vec3 center3 = glm::normalize(inDir3 + inDir1);

		DrawWireUnitSphereRecursive(matrix, inColor, inDir1, center1, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center1, center2, center3, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center1, inDir2, center2, inLevel - 1);
		DrawWireUnitSphereRecursive(matrix, inColor, center3, center2, inDir3, inLevel - 1);
	}
}
