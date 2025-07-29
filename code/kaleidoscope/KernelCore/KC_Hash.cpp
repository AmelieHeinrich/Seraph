//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 13:21:40
//

#include "KC_Hash.h"

namespace KC
{
    size_t Hash(const KC::String& str)
    {
        // FNV-1a hash
        const size_t fnv_prime = 0x100000001B3;
        const size_t fnv_offset = 0xcbf29ce484222325;
        
        size_t hash = fnv_offset;
        for (char c : str)
        {
            hash ^= static_cast<size_t>(c);
            hash *= fnv_prime;
        }
        return hash;
    }
}
