#include "timer.hpp"

namespace expengine {

double Timer::getPercentage()
{
    if (duration_.count() > 0)
    {
        return std::max(1.0, getElapsedTime() / duration_.count());
    }
    else
    {
        return 1.0;
    }
}

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
