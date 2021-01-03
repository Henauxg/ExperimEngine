#include "WebGPURenderer.hpp"

#include <random>

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
    mainWindow_ = std::make_shared<Window>(windowWidth, windoHeight, appName, 0);

    /* TODO implement */
}

WebGPURenderer::~WebGPURenderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGPU renderer destruction");
}

void WebGPURenderer::render()
{
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(10, 17);
    std::this_thread::sleep_for(std::chrono::milliseconds(uni(rng)));
}

void WebGPURenderer::handleEvent(const SDL_Event& event)
{ /* TODO implement */
}

void WebGPURenderer::rendererWaitIdle()
{ /* TODO implement */
}

std::shared_ptr<Window> WebGPURenderer::getMainWindow() { return mainWindow_; }

} // namespace wgpu
} // namespace render
} // namespace expengine
