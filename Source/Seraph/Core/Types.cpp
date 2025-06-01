//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-01 14:18:51
//

#include "Types.h"
#include <Windows.h>

bool IsPointerValid(void* ptr)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
        return false;

    return mbi.State == MEM_COMMIT;;
}

void SafeMemcpy(void* dst, const void* src, uint64 size)
{
    if (dst == nullptr || !IsPointerValid(dst))
        return;
    memcpy(dst, src, size);
}
