//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-31 14:19:56
//

#pragma once

#include <Core/Context.h>

enum class RHISamplerAddress
{
    kWrap,
    kMirror,
    kClamp,
    kBorder
};

enum class RHISamplerFilter
{
    kLinear,
    kNearest
};

struct RHISamplerDesc
{
    RHISamplerAddress Address;
    RHISamplerFilter Filter;
    bool UseMips = false;

    RHISamplerDesc() = default;
    RHISamplerDesc(RHISamplerAddress a, RHISamplerFilter f, bool m)
        : Address(a), Filter(f), UseMips(m) {}
};

class IRHISampler
{
public:
    ~IRHISampler() = default;

    RHISamplerDesc GetDesc() const { return mDesc; }

protected:
    RHISamplerDesc mDesc;
};
