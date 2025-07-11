//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-07-11 11:40:00
//

#include "Core/Log.h"
#include <iostream>

// ANSI color codes
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"

void ConsoleLogger::Output(LogLevel level, const String& format)
{
    const char* colorCode = ANSI_COLOR_RESET;

    switch (level)
    {
        case LogLevel::kInfo:     colorCode = ANSI_COLOR_GREEN; break;
        case LogLevel::kWarn:     colorCode = ANSI_COLOR_YELLOW; break;
        case LogLevel::kError:    colorCode = ANSI_COLOR_RED; break;
        case LogLevel::kFatal:    colorCode = ANSI_COLOR_MAGENTA; break;
        case LogLevel::kWhatever: colorCode = ANSI_COLOR_CYAN; break;
    }

    std::cout << colorCode << format << ANSI_COLOR_RESET << std::endl;
}
