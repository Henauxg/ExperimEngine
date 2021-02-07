#pragma once

#include <string>

#ifdef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif // DEBUG

#include <spdlog/spdlog.h>

namespace experim {

const std::string LOGGER_NAME = "expengine_main_logger";

// TODO Cross-platform
const std::string LOG_DIRECTORY = "logs";
const std::string LOG_FILE = LOG_DIRECTORY + "/expengine_log.txt";

#define EXPENGINE_ASSERT(f, ...)                                                    \
    do                                                                              \
    {                                                                               \
        bool expr = (f);                                                            \
        if (!expr)                                                                  \
        {                                                                           \
            SPDLOG_ERROR("Fatal : assert failed. Program will abort.");             \
            SPDLOG_ERROR(__VA_ARGS__);                                              \
            abort();                                                                \
        }                                                                           \
    } while (0)

#ifndef NDEBUG
#define EXPENGINE_DEBUG_ASSERT(f, ...) EXPENGINE_ASSERT(f, __VA_ARGS__)
#else
#define EXPENGINE_DEBUG_ASSERT(f, ...)                                              \
    do                                                                              \
    {                                                                               \
    } while (0)
#endif
} // namespace experim
