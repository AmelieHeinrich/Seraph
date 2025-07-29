//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 21:00:34
//

#include "KC_Math.h"

namespace KC
{
    KC::Array<glm::vec4> Math::GetFrustumCorners(glm::mat4 view, glm::mat4 proj)
    {
        glm::mat4 inv = glm::inverse(proj * view);

        KC::Array<glm::vec4> corners = {
            glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f),
            glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f),
            glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f),
            glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
            glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f),
            glm::vec4( 1.0f,  1.0f, 1.0f, 1.0f),
            glm::vec4( 1.0f, -1.0f, 1.0f, 1.0f),
            glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
        };

        // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
        for (int i = 0; i < 8; i++) {
            glm::vec4 h = inv * corners[i];
            h.x /= h.w;
            h.y /= h.w;
            h.z /= h.w;
            corners[i] = h;
        }
        return corners;
    }
}
