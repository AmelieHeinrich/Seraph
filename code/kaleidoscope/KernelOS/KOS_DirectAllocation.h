//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 15:39:10
//

#pragma once

#ifdef KD_WINDOWS
    #include "Windows/KOS_DirectionAllocation.Windows.h"
#endif

#include <memory>

namespace KOS
{
    template<typename T, typename... Args>
    T* New(Args&&... args)
    {
        constexpr uint64 size = sizeof(T);
        constexpr uint64 align = alignof(T);
        
        void* mem;
        if constexpr (align > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
            mem = AllocateMem(size, align);
        } else {
            mem = AllocateMem(size);
        }
        return std::construct_at(reinterpret_cast<T*>(mem), std::forward<Args>(args)...);
    }
    
    template<typename T>
    T* NewArray(uint count)
    {
        constexpr uint64 size = sizeof(T);
        constexpr uint64 align = alignof(T);
        
        void* mem = AllocateMem(size * count, align);
        T* ptr = reinterpret_cast<T*>(mem);
        
        std::uninitialized_default_construct_n(ptr, count);
        return ptr;
    }
    
    template<typename T>
    void Delete(T* ptr)
    {
        if (ptr) {
            std::destroy_at(ptr);  // Call destructor
            FreeMem(ptr);          // Free memory
        }
    }
    
    template<typename T>
    void DeleteArray(T* ptr, uint count)
    {
        if (ptr) {
            // Use std::destroy for exception safety
            std::destroy_n(ptr, count);
            FreeMem(ptr);
        }
    }
}
