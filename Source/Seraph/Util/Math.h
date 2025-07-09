//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-14 16:39:47
//

#pragma once

#include <Core/Types.h>

#include <glm/glm.hpp>

struct FrustumPlane
{
    float3 Normal;
    float Distance;
};

namespace Math
{
    Array<float4> GetFrustumCorners(glm::mat4 view, glm::mat4 proj);
}
