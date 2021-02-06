#pragma once

#include <chrono>

using Seconds = std::ratio<1>;
using Milliseconds = std::ratio<1, 1000>;
using Microseconds = std::ratio<1, 1000000>;
using Nanoseconds = std::ratio<1, 1000000000>;

namespace expengine {

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using DefaultResolution = Milliseconds;

    /**
     * @brief Create a timer with the specified duration in DefaultResolution
     */
    Timer(double duration = 0)
        : start_(Clock::now())
        , duration_(std::chrono::duration<double, DefaultResolution>(duration))
        , expiredFlag_(false)
    {
    }

    /**
     * @brief Returns elapsed time in the specified unit
     */
    template <typename T = DefaultResolution> double getElapsedTime()
    {
        return std::chrono::duration<double, T>(Clock::now() - start_).count();
    }
    /**
     * @brief Returns time left in the specified unit
     */
    template <typename T = DefaultResolution> double getTimeLeft()
    {
        auto elapsed
            = std::chrono::duration<double, T>(Clock::now() - start_).count();
        auto duration
            = std::chrono::duration_cast<std::chrono::duration<double, T>>(duration_)
                  .count();
        return duration - elapsed;
    }

    /**
     * @brief Return a percentage of completion. Scale from 0 to 1.
     */
    double getPercentage();

    /**
     * @brief Returns true if more time than duration has elapsed
     */
    bool isExpired();
    /**
     * @brief Returns true after isExpired is true only once when first called.
     */
    bool justExpiredFlag();

    /**
     * @brief Restarts the timer.
     */
    void reset();

private:
    Clock::time_point start_;
    std::chrono::duration<double, DefaultResolution> duration_;
    bool expiredFlag_;
};

} // namespace expengine