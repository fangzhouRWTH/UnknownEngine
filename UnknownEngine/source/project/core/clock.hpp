#pragma once
#include <chrono>

namespace unknown::core
{
    class Clock
    {
    public:
        Clock();
        ~Clock();
        void Activate();
        std::chrono::duration<double> CheckDeltaTime();
        std::chrono::duration<double> GetDeltaTime() const;
        std::chrono::duration<double> CheckTimePast();

    private:
        bool bActive = false;
        std::chrono::high_resolution_clock::time_point mStart;
        std::chrono::high_resolution_clock::time_point mLastCheck;
        std::chrono::duration<double> mDelta;
    };
}