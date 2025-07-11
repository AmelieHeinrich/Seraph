//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:18:51
//

#include "Types.h"

void SafeMemcpy(void* dst, const void* src, uint64 size)
{
    if (dst && src)
        memcpy(dst, src, size);
}
