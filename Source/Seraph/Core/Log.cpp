//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 07:16:56
//

#include "Log.h"

#include <iostream>
#include <sstream>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <Windows.h>

void ILogger::Log(LogLevel level, const char* file, int line, const char* fmt, va_list args)
{
    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);  // Format the log message

    std::ostringstream stream;
    stream << "[" << GetTimeString() << "] "
           << "[" << file << ":" << line << "] "
           << "[" << LevelToString(level) << "] "
           << buffer;
    
    Output(level, stream.str());  // Output the formatted message
}

void ILogger::Whatever(const char* file, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::kWhatever, file, line, fmt, args);
    va_end(args);
}

void ILogger::Info(const char* file, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::kInfo, file, line, fmt, args);
    va_end(args);
}

void ILogger::Warn(const char* file, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::kWarn, file, line, fmt, args);
    va_end(args);
}

void ILogger::Error(const char* file, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::kError, file, line, fmt, args);
    va_end(args);
}

void ILogger::Fatal(const char* file, int line, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    Log(LogLevel::kFatal, file, line, fmt, args);
    va_end(args);
}

String ILogger::LevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::kInfo: return "Info";
        case LogLevel::kWarn: return "Warn";
        case LogLevel::kError: return "Error";
        case LogLevel::kFatal: return "Fatal";
        case LogLevel::kWhatever: return "Whatevs";
        default: return "Dafuuuk";
    }
}

String ILogger::GetTimeString()
{
    std::time_t now = std::time(nullptr);
    std::tm tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void ConsoleLogger::Output(LogLevel level, const String& format)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color = 7; // Default gray/white
    
    switch (level)
    {
        case LogLevel::kInfo: color = 10; break;        // Green
        case LogLevel::kWarn: color = 14; break;        // Yellow
        case LogLevel::kError: color = 12; break;       // Red
        case LogLevel::kFatal: color = 13; break;       // Magenta
        case LogLevel::kWhatever: color = 11; break;    // Cyan
    }
    
    SetConsoleTextAttribute(hConsole, color);
    std::cout << format << std::endl;
    SetConsoleTextAttribute(hConsole, 7);
}

FileLogger::FileLogger(const String& path)
{
    mStream.open(path, std::ios::trunc | std::ios::out);
    if (!mStream.is_open()) {
        Output(LogLevel::kFatal, "Failed to open output file!");
    }
}

FileLogger::~FileLogger()
{
    mStream.close();
}

void FileLogger::Output(LogLevel level, const String& format)
{
    mStream << format << std::endl;
}

MultiLogger::~MultiLogger()
{
    for (ILogger* logger : mLoggers) {
        delete logger;
    }
    mLoggers.clear();
}
    
void MultiLogger::AddLogger(ILogger* logger)
{
    mLoggers.push_back(logger);
}

void MultiLogger::Output(LogLevel level, const String& format)
{
    for (ILogger* logger : mLoggers) {
        logger->Output(level, format);
    }
}
