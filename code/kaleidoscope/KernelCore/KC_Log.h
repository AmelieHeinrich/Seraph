//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 13:44:50
//

#pragma once

#include <cstdarg>

#include "KC_Common.h"
#include "KC_Array.h"
#include "KC_String.h"
#include "KC_FileStream.h"

namespace KC
{
    enum class LogLevel
    {
        kWhatever,
        kInfo,
        kWarn,
        kError,
        kFatal,
    };
    
    class ILogger
    {
    public:
        virtual ~ILogger() = default;
    
        void Log(LogLevel level, const char* file, int line, const char* format, va_list arguments);
        void Whatever(const char* file, int line, const char* format, ...);
        void Info(const char* file, int line, const char* format, ...);
        void Warn(const char* file, int line, const char* format, ...);
        void Error(const char* file, int line, const char* format, ...);
        void Fatal(const char* file, int line, const char* format, ...);
    
        virtual void Output(LogLevel level, const String& format) = 0;
    private:
        String LevelToString(LogLevel level);
        String GetTimeString();
    };
    
    class FileLogger : public ILogger
    {
    public:
        FileLogger(const String& path);
        ~FileLogger();
    
    protected:
        virtual void Output(LogLevel level, const String& format) override;
    
        FileStream mStream;
    };
    
    class ConsoleLogger : public ILogger
    {
    public:
        ConsoleLogger() = default;
        ~ConsoleLogger() = default;
    
    protected:
        virtual void Output(LogLevel level, const String& format) override;
    };
    
    class MultiLogger : public ILogger
    {
    public:
        MultiLogger() = default;
        ~MultiLogger();
        
        void AddLogger(ILogger* logger);
    protected:
        virtual void Output(LogLevel level, const String& format) override;
    
        Array<ILogger*> mLoggers;
    };
}
