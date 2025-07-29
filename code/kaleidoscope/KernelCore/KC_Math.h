//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 20:59:42
//

#pragma once

#include "KC_Context.h"

#include <glm/glm.hpp>

namespace KC
{
    struct FrustumPlane
    {
        glm::vec3 Normal;
        float Distance;
    };

    class Math
    {
    public:
        static KC::Array<glm::vec4> GetFrustumCorners(glm::mat4 view, glm::mat4 proj);
    };
}
