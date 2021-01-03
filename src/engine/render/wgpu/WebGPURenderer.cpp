#include "WebGPURenderer.hpp"

#include <ExperimEngineConfig.h>

namespace {

}

namespace expengine {
namespace render {
namespace wgpu {

WebGPURenderer::WebGPURenderer(
    const std::string& appName,
    const uint32_t appVersion,
    int windowWidth,
    int windoHeight,
    EngineParameters& engineParams)
    : Renderer(engineParams)
{
    /* TODO implement */
}

WebGPURenderer::~WebGPURenderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGPU renderer destruction");
}

void WebGPURenderer::render()
{ /* TODO Implement */
}

void WebGPURenderer::handleEvent(const SDL_Event& event)
{ /* TODO implement */
}

void WebGPURenderer::rendererWaitIdle()
{ /* TODO implement */
}

std::shared_ptr<Window> WebGPURenderer::getMainWindow()
{
    /* TODO implement */
    return std::make_shared<Window>(10,10,"No title", 0);
}

} // namespace wgpu
} // namespace render
} // namespace expengine
