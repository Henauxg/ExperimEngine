#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace expengine {
namespace render {

RenderingContext::RenderingContext()
    : logger_(spdlog::get(LOGGER_NAME)) {};

} // namespace render
} // namespace expengine
