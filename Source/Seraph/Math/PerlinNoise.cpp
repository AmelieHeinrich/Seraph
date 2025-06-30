//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-30 21:46:08
//

#include "PerlinNoise.h"

#include <cmath>
#include <algorithm>

PerlinNoise::PerlinNoise(uint32_t seed)
{
    // Initialize permutation with 0..255
    for (int i = 0; i < 256; ++i) {
        p[i] = i;
    }

    // Shuffle using a simple LCG based on seed
    for (int i = 255; i > 0; --i) {
        seed = seed * 1664525u + 1013904223u;
        int j = seed % (i + 1);
        std::swap(p[i], p[j]);
    }

    // Duplicate the array to avoid overflow in indexing
    for (int i = 0; i < 256; ++i) {
        p[256 + i] = p[i];
    }
}

float PerlinNoise::Noise(float x, float y) const
{
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);

    float u = Fade(x);
    float v = Fade(y);

    int aa = p[p[X] + Y];
    int ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y];
    int bb = p[p[X + 1] + Y + 1];

    return Lerp(v,
        Lerp(u, Grad(aa, x, y),     Grad(ba, x - 1, y)),
        Lerp(u, Grad(ab, x, y - 1), Grad(bb, x - 1, y - 1))
    );
}

float PerlinNoise::Fade(float t)
{
    // 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::Lerp(float t, float a, float b)
{
    return a + t * (b - a);
}

float PerlinNoise::Grad(int hash, float x, float y)
{
    switch (hash & 3) {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        default: return 0.0f; // never happens
    }
}
