//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 06:55:27
//

#pragma once

#include <cstdarg>
#include <fstream>
#include <vector>

#include "Types.h"

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

    std::ofstream mStream;
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
