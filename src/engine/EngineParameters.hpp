#pragma once

#include <cstdint>

#include <engine/utils/Timer.hpp>

namespace {
const double DEFAULT_FPS_REFRESH_PERIOD = 1000.0;
}

namespace experim {

struct GraphicSettings {
};

struct EngineTimings {
    /** @brief Last frame duration (in milliseconds) */
    double frameDuration = 0.0;
    /** @brief Defines a frame rate independent timer value clamped from 0
     * to 1.0. */
    float timer = 0.0f;
    /** @brief Multiplier for speeding up (or slowing down) the global
     * timer */
    float timerSpeed = 1.0f;
    /** @brief Used to pause the global timer */
    bool paused = false;
};

struct EngineStatistics {
    /** @brief Timer used to compute the FPS. */
    Timer fpsTimer;
    /** @brief Period (in ms) used to compute the average FPS value. */
    float fpsRefreshPeriod = DEFAULT_FPS_REFRESH_PERIOD;
    /** @brief Number of frames since the last FPS timer refresh. */
    uint32_t frameCounter = 0;
    /** @brief FPS value during the last second. */
    uint32_t fpsValue = 0;

    EngineStatistics()
        : fpsTimer(DEFAULT_FPS_REFRESH_PERIOD)
    {
    }
};

struct EngineParameters {
    EngineStatistics statistics;
    EngineTimings timings;
};

} // namespace experim
