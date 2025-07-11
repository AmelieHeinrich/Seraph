//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:35:03
//

#include "Core/Assert.h"
#include "Core/Context.h"

#include <iostream>
#include <csignal>
#include <cstdlib>

void Assert::Eq(bool condition, const std::string& file, const std::string& function, int line, const std::string& message)
{
    if (!condition) {
        SERAPH_FATAL(file.c_str(), line, "ASSERTION FAILED (%s:%s - line %d): %s", file.c_str(), function.c_str(), line, message.c_str());

        // Print to stderr
        std::cerr << "Assertion Failed!\n";
        std::cerr << "File: " << file << "\nFunction: " << function << "\nLine: " << line << "\nMessage: " << message << std::endl;

        // Optional: pop up a macOS alert using AppleScript
        std::string command = "osascript -e 'tell app \"System Events\" to display dialog \"Assertion Failed!\\n" + message + "\" with title \"SERAPH\" buttons {\"OK\"} with icon stop'";
        system(command.c_str());

        // Trigger a debug trap
        raise(SIGTRAP);
    }
}