#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace expengine {
namespace render {

RenderingContext::RenderingContext(std::function<void(void)> surfaceChangeCallback)
    : surfaceChangeCallback_(surfaceChangeCallback)
    , logger_(spdlog::get(LOGGER_NAME)) {};

void RenderingContext::setSurfaceChangeCallback(
    std::function<void(void)> surfaceChangeCallback)
{
    surfaceChangeCallback_ = surfaceChangeCallback;
}

} // namespace render
} // namespace expengine
