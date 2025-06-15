//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-13 21:52:02
//

#pragma once

#include <random>
#include <glm/glm.hpp>

class Random
{
public:
    Random()
        : rng(std::random_device{}()) {}

    Random(uint seed)
        : rng(seed) {}

    float Float(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    float2 Vec2(const float2& min, const float2& max) {
        return float2(
            Float(min.x, max.x),
            Float(min.y, max.y)
        );
    }

    float3 Vec3(const float3& min, const float3& max) {
        return float3(
            Float(min.x, max.x),
            Float(min.y, max.y),
            Float(min.z, max.z)
        );
    }

    float4 Vec4(const float4& min, const float4& max) {
        return float4(
            Float(min.x, max.x),
            Float(min.y, max.y),
            Float(min.z, max.z),
            Float(min.w, max.w)
        );
    }

private:
    std::mt19937 rng;
};
