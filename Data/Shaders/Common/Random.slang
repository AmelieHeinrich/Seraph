//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-24 07:59:47
//

module Random;

uint rng_rotl(uint x, uint k)
{
    return (x << k) | (x >> (32 - k));
}

uint rng_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

public struct RNG
{
    uint2 s;

    [mutating]
    public void Init(uint2 id, uint frameIndex)
    {
        uint s0 = (id.x << 16) | id.y;
        uint s1 = frameIndex;

        s.x = rng_hash(s0);
        s.y = rng_hash(s1);
        Next();
    }

    [mutating]
    public uint Next()
    {
        uint result = s.x * 0x9e3779bb;

        s.y ^= s.x;
        s.x = rng_rotl(s.x, 26) ^ s.y ^ (s.y << 9);
        s.y = rng_rotl(s.y, 13);

        return result;
    }

    [mutating]
    public float NextFloat()
    {
        uint u = 0x3f800000 | (Next() >> 9);
        return asfloat(u) - 1.0;
    }

    [mutating]
    public uint NextUint(uint max)
    {
        float f = NextFloat();
        return uint(floor(f * max));
    }

    [mutating]
    public float2 NextFloat2()
    {
        return float2(NextFloat(), NextFloat());
    }

    [mutating]
    public float3 NextFloat3()
    {
        return float3(NextFloat(), NextFloat(), NextFloat());
    }

    [mutating]
    public float3 NextUnitVector()
    {
        float z = 1.0 - 2.0 * NextFloat();
        float phi = 2.0 * 3.14159 * NextFloat();
        float r = sqrt(1.0 - z * z);

        return float3(r * cos(phi), r * sin(phi), z);
    }

    [mutating]
    public float3 NextHemisphereUnitVector(float3 normal)
    {
        float3 onUnitSphere = NextUnitVector();
        if (dot(onUnitSphere, normal) > 0.0)
            return onUnitSphere;
        return -onUnitSphere;
    }
};
