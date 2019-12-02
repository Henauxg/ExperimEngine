#pragma once

#include <cstdint>

namespace experimengine {
struct GraphicSettings {
};

struct EngineTimings {
	/** @brief Last frame duration (in seconds) measured using a high performance timer (if
	 * available) */
	float frameDuration = 0.0f;
	/** @brief Defines a frame rate independent timer value clamped from 0 to 1.0. */
	float timer = 0.0f;
	/** @brief Multiplier for speeding up (or slowing down) the global timer */
	float timerSpeed = 1.0f;
	/** @brief Used to pause the global timer */
	bool paused = false;
};

struct EngineStatistics {
	/** @brief Timer used to compute the FPS. */
	float fpsTimer = 0.0f;
	/** @brief Period (in ms) used to compute the average FPS value. */
	float fpsRefreshPeriod = 50.0f;
	/** @brief Number of frames since the last FPS timer refresh. */
	uint32_t frameCounter = 0;
	/** @brief FPS value during the last second. */
	uint32_t fpsValue = 0;
};

struct EngineParameters {
	EngineStatistics statistics;
	EngineTimings timings;
};
} // namespace experimengine