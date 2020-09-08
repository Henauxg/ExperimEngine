#pragma once

#include <string>

#ifdef DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#endif // DEBUG

#include <spdlog/spdlog.h>

namespace expengine {

const std::string LOGGER_NAME = "expengine_main_logger";

const std::string LOG_DIRECTORY = "logs";
const std::string LOG_FILE = LOG_DIRECTORY + "/expengine_log.txt";

} // namespace expengine