#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace experim {

RenderingContext::RenderingContext(std::function<void(void)> surfaceChangeCallback)
    : surfaceChangeCallback_(surfaceChangeCallback)
    , logger_(spdlog::get(LOGGER_NAME))
    , frameToSubmit_(false) {};

void RenderingContext::setSurfaceChangeCallback(
    std::function<void(void)> surfaceChangeCallback)
{
    surfaceChangeCallback_ = surfaceChangeCallback;
}

} // namespace experim
