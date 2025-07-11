//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:33:18
//

#pragma once

#include "Log.h"

class Assert
{
public:
    static void Eq(bool condition, const std::string& file, const std::string& function, int line, const std::string& message);
};

#define ASSERT_EQ(cond, msg) Assert::Eq(cond, __FILE__, __FUNCTION__, __LINE__, msg);
