//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-30 21:45:50
//

#pragma once

#include <cstdint>

class PerlinNoise
{
public:
    explicit PerlinNoise(uint32_t seed = 0);

    float Noise(float x, float y) const;

    static float Fade(float t);
    static float Lerp(float t, float a, float b);
    static float Grad(int hash, float x, float y);
private:
    int p[512];
};
