#include "WGpuRenderingContext.hpp"

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu_cpp.h>
#endif

#include <engine/log/ExpengineLog.hpp>

namespace {

}

namespace expengine {
namespace render {
namespace webgpu {

WebGpuRenderingContext::WebGpuRenderingContext(
    const wgpu::Device& device,
    std::shared_ptr<Window> window,
    AttachmentsFlags attachmentFlags)
    : RenderingContext()
    , window_(window)
    , device_(device)
    , attachmentsFlags_(attachmentFlags)
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuRenderingContext creation");

    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

WebGpuRenderingContext::~WebGpuRenderingContext()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuRenderingContext destruction");
}

void WebGpuRenderingContext::handleSurfaceChanges()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

void WebGpuRenderingContext::beginFrame()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

void WebGpuRenderingContext::submitFrame()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

std::shared_ptr<RenderingContext> WebGpuRenderingContext::clone(
    std::shared_ptr<Window> window,
    AttachmentsFlags attachmentFlags)
{
    return std::make_shared<WebGpuRenderingContext>(
        device_, window, attachmentFlags);
}

} // namespace webgpu
} // namespace render
} // namespace expengine
