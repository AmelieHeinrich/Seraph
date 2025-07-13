//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 07:33:28
//

#pragma once

#include "Log.h"
#include "Assert.h"

struct Context
{
    static void Initialize();
    static void Shutdown();

    static Context sContext;

    ILogger* logger;
};

#if SERAPH_ENABLE_LOGGING
    #define SERAPH_INFO(fmt, ...)         Context::sContext.logger->Info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define SERAPH_WARN(fmt, ...)         Context::sContext.logger->Warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define SERAPH_ERROR(fmt, ...)        Context::sContext.logger->Error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #define SERAPH_FATAL(fmt, ...)        Context::sContext.logger->Fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

    #ifndef NDEBUG
        #define SERAPH_WHATEVER(fmt, ...) Context::sContext.logger->Whatever(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
    #else
        #define SERAPH_WHATEVER(fmt, ...)
    #endif
#else
    #define SERAPH_INFO(fmt, ...)
    #define SERAPH_WARN(fmt, ...)
    #define SERAPH_ERROR(fmt, ...)
    #define SERAPH_FATAL(fmt, ...)
    #define SERAPH_WHATEVER(fmt, ...)
#endif
