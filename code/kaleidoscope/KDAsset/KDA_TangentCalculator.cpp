//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 17:30:36
//

#include "KDA_TangentCalculator.h"
#include "KDA_MeshLoader.h"

#include <mikktspace.h>

namespace MikkT
{
    int getNumFaces(const SMikkTSpaceContext* context)
    {
        auto mesh = static_cast<KDA::MeshPrimitive*>(context->m_pUserData);
        return static_cast<int>(mesh->Indices.size() / 3);
    }

    int getNumVerticesOfFace(const SMikkTSpaceContext* context, int faceIdx)
    {
        return 3;
    }

    void getPosition(const SMikkTSpaceContext* context, float outpos[], int faceIdx, int vertIdx)
    {
        auto mesh = static_cast<KDA::MeshPrimitive*>(context->m_pUserData);
        auto& pos = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Position;
        outpos[0] = pos.x;
        outpos[1] = pos.y;
        outpos[2] = pos.z;
    }

    void getNormal(const SMikkTSpaceContext* context, float outnormal[], int faceIdx, int vertIdx)
    {
        auto mesh = static_cast<KDA::MeshPrimitive*>(context->m_pUserData);
        auto& normal = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Normal;
        outnormal[0] = normal.x;
        outnormal[1] = normal.y;
        outnormal[2] = normal.z;
    }

    void getTexCoord(const SMikkTSpaceContext* context, float outuv[], int faceIdx, int vertIdx)
    {
        auto mesh = static_cast<KDA::MeshPrimitive*>(context->m_pUserData);
        auto& uv = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].UV;
        outuv[0] = uv.x;
        outuv[1] = uv.y;
    }

    void setTSpaceBasic(const SMikkTSpaceContext* context, const float inTangent[], float sign, int faceIdx, int vertIdx)
    {
        auto mesh = static_cast<KDA::MeshPrimitive*>(context->m_pUserData);
        auto& tangent = mesh->Vertices[mesh->Indices[faceIdx * 3 + vertIdx]].Tangent;
        tangent.x = inTangent[0];
        tangent.y = inTangent[1];
        tangent.z = inTangent[2];
        tangent.w = -1.0f * sign;
    }
}

namespace KDA
{
    void TangentCalculator::ComputeTangents(MeshPrimitive* primitive)
    {
        SMikkTSpaceInterface mikktInterface = {};
        mikktInterface.m_getNumFaces = &MikkT::getNumFaces;
        mikktInterface.m_getNumVerticesOfFace = &MikkT::getNumVerticesOfFace;
        mikktInterface.m_getPosition = &MikkT::getPosition;
        mikktInterface.m_getNormal = &MikkT::getNormal;
        mikktInterface.m_getTexCoord = &MikkT::getTexCoord;
        mikktInterface.m_setTSpaceBasic = &MikkT::setTSpaceBasic;

        SMikkTSpaceContext context = {};
        context.m_pInterface = &mikktInterface;
        context.m_pUserData = primitive;
        if (genTangSpaceDefault(&context) == false) {
            for (auto& vertex : primitive->Vertices) {
                vertex.Tangent = KGPU::float4(1.0f, 0.0f, 0.0f, 1.0f);
            }
        }
    }
}
