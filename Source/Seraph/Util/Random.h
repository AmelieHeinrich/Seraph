//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 21:52:02
//

#pragma once

#include <random>
#include <glm/glm.hpp>

class Random {
public:
    Random()
        : rng(std::random_device{}()) {}

    float Float(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    glm::vec2 Vec2(const glm::vec2& min, const glm::vec2& max) {
        return glm::vec2(
            Float(min.x, max.x),
            Float(min.y, max.y)
        );
    }

    glm::vec3 Vec3(const glm::vec3& min, const glm::vec3& max) {
        return glm::vec3(
            Float(min.x, max.x),
            Float(min.y, max.y),
            Float(min.z, max.z)
        );
    }

    glm::vec4 Vec4(const glm::vec4& min, const glm::vec4& max) {
        return glm::vec4(
            Float(min.x, max.x),
            Float(min.y, max.y),
            Float(min.z, max.z),
            Float(min.w, max.w)
        );
    }

private:
    std::mt19937 rng;
};
