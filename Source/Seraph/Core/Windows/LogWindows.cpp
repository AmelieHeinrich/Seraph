//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-07-11 11:40:00
//

#include "Core/Log.h"

#include <Windows.h>
#include <iostream>

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
