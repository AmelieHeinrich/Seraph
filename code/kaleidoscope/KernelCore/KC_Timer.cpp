//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 11:35:21
//

#include "KC_Timer.h"

namespace KC
{
    Timer GlobalTimer;

    Timer::Timer()
    {
        Restart();
    }

    void Timer::Restart()
    {
        mStart = Clock::now();
    }

    std::chrono::duration<double> Timer::GetElapsed() const
    {
        return (Clock::now() - mStart);
    }

    double Timer::ToMiliseconds() const
    {
        return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(GetElapsed()).count();
    }

    double Timer::ToSeconds() const
    {
        return GetElapsed().count();
    }
}
