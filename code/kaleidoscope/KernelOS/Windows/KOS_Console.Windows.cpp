//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 16:19:35
//

#include "KOS_Console.h"

#include <Windows.h>

namespace KOS {
    void Console::SetColor(ConsoleColor color)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) return;
        
        WORD attributes = 0;
        
        switch (color)
        {
        case ConsoleColor::kDefault:
            attributes = 7; // White (default)
            break;
        case ConsoleColor::kRed:
            attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
            break;
        case ConsoleColor::kGreen:
            attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case ConsoleColor::kBlue:
            attributes = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        case ConsoleColor::kYellow:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            break;
        case ConsoleColor::kWhite:
            attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            break;
        }
    
        SetConsoleTextAttribute(hConsole, attributes);
    }

    void Console::Write(const char* data, uint64 size)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole == INVALID_HANDLE_VALUE) return;

        WriteConsoleA(hConsole, data, size, nullptr, nullptr);
        OutputDebugStringA(data);
    }
}
