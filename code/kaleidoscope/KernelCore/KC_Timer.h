//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-19 11:33:12
//

#pragma once

#include <chrono>

namespace KC
{
    class Timer
    {
    public:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;

        Timer();

        void Restart();
        std::chrono::duration<double> GetElapsed() const;
        
        double ToMiliseconds() const;
        double ToSeconds() const;
    private:
        TimePoint mStart;
    };

    extern Timer GlobalTimer;
}
