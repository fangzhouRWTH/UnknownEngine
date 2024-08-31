#include "clock.hpp"
#include "cassert"

namespace unknown::core
{
    Clock::Clock()
    {
    }

    Clock::~Clock()
    {
    }

    void Clock::Activate()
    {
        bActive = true;
        mStart = std::chrono::high_resolution_clock::now();
        mLastCheck = mStart;
    }

    std::chrono::duration<double> Clock::CheckDeltaTime()
    {
        assert(bActive);
        auto nowTime = std::chrono::high_resolution_clock::now();
        mDelta = nowTime - mLastCheck;
        mLastCheck = nowTime;

        return mDelta;
    }
    std::chrono::duration<double> Clock::GetDeltaTime() const
    {
        assert(bActive);
        return mDelta;
    }
    std::chrono::duration<double> Clock::CheckTimePast()
    {
        assert(bActive);
        return std::chrono::high_resolution_clock::now() - mStart;
    }
}