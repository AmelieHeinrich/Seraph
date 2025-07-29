//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 13:46:45
//

#include "KC_Log.h"
#include "KC_Context.h"

#include <KernelOS/KOS_DirectAllocation.h>
#include <KernelOS/KOS_Console.h>

#include <cstdarg>
#include <ctime>

namespace KC {
    void ILogger::Log(LogLevel level, const char* file, int line, const char* fmt, va_list args)
    {
        char buffer[2048];
        vsnprintf(buffer, sizeof(buffer), fmt, args);  // Format the log message

        String out = StringUtil::Format("[%s] [%s] [%s:%d] %s", LevelToString(level).c_str(), GetTimeString().c_str(), file, line, buffer);
        Output(level, out);
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
        
        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

        return buffer;
    }

    FileLogger::FileLogger(const String& path)
    {
        mStream.Open(path.c_str(), KOS::FileOpenMode::kWrite | KOS::FileOpenMode::kOverwrite);
        if (!mStream.IsOpen()) {
            Output(LogLevel::kFatal, "Failed to open output file!");
        }
    }

    FileLogger::~FileLogger()
    {
        mStream.Close();
    }

    void FileLogger::Output(LogLevel level, const String& format)
    {
        mStream.Write(format);
        mStream.Write("\n", 1);
    }

    void ConsoleLogger::Output(LogLevel level, const String& format)
    {
        KOS::ConsoleColor color = KOS::ConsoleColor::kDefault;
        switch (level)
        {
            case LogLevel::kInfo: color = KOS::ConsoleColor::kGreen; break;
            case LogLevel::kWarn: color = KOS::ConsoleColor::kYellow; break;
            case LogLevel::kError: color = KOS::ConsoleColor::kRed; break;
            case LogLevel::kFatal: color = KOS::ConsoleColor::kRed; break;
            case LogLevel::kWhatever: color = KOS::ConsoleColor::kBlue; break;
        }

        KOS::Console::SetColor(color);
        KOS::Console::Write(format.c_str(), format.size());
        KOS::Console::Write("\n", 1);
        KOS::Console::SetColor(KOS::ConsoleColor::kDefault);
    }

    MultiLogger::~MultiLogger()
    {
        for (ILogger* logger : mLoggers) {
            KC_DELETE(logger);
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
}
