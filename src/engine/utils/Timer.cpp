#include "timer.hpp"

namespace expengine {

double Timer::getPercentage() { return getElapsedTime() / duration_.count(); }

bool Timer::isExpired() { return getTimeLeft() <= 0; }

bool Timer::justExpiredFlag()
{
    if (isExpired() && !expiredFlag_)
    {
        expiredFlag_ = true;
        return true;
    }
    else
    {
        return false;
    }
}

void Timer::reset()
{
    expiredFlag_ = false;
    start_ = Clock::now();
}

} // namespace expengine
