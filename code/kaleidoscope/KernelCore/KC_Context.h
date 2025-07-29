//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:00:35
//

#pragma once

#include "KC_Log.h"
#include "KC_Assert.h"
#include "KC_Allocator.h"

#define KD_MEMORY_ALLOC(size)      KC::Context::sContext.allocator->Allocate(size, __LINE__, __FILE__)
#define KD_MEMORY_FREE(block)      KC::Context::sContext.allocator->Free(block)

// Memory allocation macros
#define KC_NEW(Type, ...)          KC::NewImpl<Type>(__LINE__, __FILE__, ##__VA_ARGS__)
#define KC_NEW_ARRAY(Type, count)  KC::NewArrayImpl<Type>(count, __LINE__, __FILE__)
#define KC_DELETE(ptr)             KC::DeleteImpl(ptr, __LINE__, __FILE__)
#define KC_DELETE_ARRAY(ptr, count) KC::DeleteArrayImpl(ptr, count, __LINE__, __FILE__)

namespace KC
{
    struct Context
    {
        static void Initialize();
        static void Shutdown();

        static Context sContext;

        ILogger* logger;
        IMemoryAllocator* allocator;
    };

    struct ScopedContext
    {
        ScopedContext() { Context::Initialize(); }
        ~ScopedContext() { Context::Shutdown(); }
    };

    template<typename T, typename... Args>
    T* NewImpl(int line, const char* file, Args&&... args)
    {
        void* mem = Context::sContext.allocator->Allocate(sizeof(T), line, file);
        return std::construct_at(reinterpret_cast<T*>(mem), std::forward<Args>(args)...);
    }
    
    template<typename T>
    T* NewArrayImpl(uint count, int line, const char* file)
    {
        void* mem = Context::sContext.allocator->Allocate(sizeof(T) * count, line, file);
        T* ptr = reinterpret_cast<T*>(mem);
        std::uninitialized_default_construct_n(ptr, count);
        return ptr;
    }
    
    template<typename T>
    void DeleteImpl(T* ptr, int line, const char* file)
    {
        if (ptr) {
            std::destroy_at(ptr);
            Context::sContext.allocator->Free(ptr);
        }
    }
    
    template<typename T>
    void DeleteArrayImpl(T* ptr, uint count, int line, const char* file)
    {
        if (ptr) {
            std::destroy_n(ptr, count);
            Context::sContext.allocator->Free(ptr);
        }
    }
}

#ifndef KD_RETAIL
    #define KD_INFO(fmt, ...)         KC::Context::sContext.logger->Info(__FILE__, __LINE__, fmt, __VA_ARGS__)
    #define KD_WARN(fmt, ...)         KC::Context::sContext.logger->Warn(__FILE__, __LINE__, fmt, __VA_ARGS__)
    #define KD_ERROR(fmt, ...)        KC::Context::sContext.logger->Error(__FILE__, __LINE__, fmt, __VA_ARGS__)
    #define KD_FATAL(fmt, ...)        KC::Context::sContext.logger->Fatal(__FILE__, __LINE__, fmt, __VA_ARGS__)
    #if defined(KD_DEBUG)
        #define KD_WHATEVER(fmt, ...) KC::Context::sContext.logger->Whatever(__FILE__, __LINE__, fmt, __VA_ARGS__)
    #else
        #define KD_WHATEVER(fmt, ...)
    #endif
#else
    #define KD_INFO(fmt, ...) 
    #define KD_WARN(fmt, ...) 
    #define KD_ERROR(fmt, ...)
    #define KD_FATAL(fmt, ...)
    #define KD_WHATEVER(fmt, ...)
#endif