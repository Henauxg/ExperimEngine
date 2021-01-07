#include "WGpuRenderer.hpp"

#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#endif

#include <ExperimEngineConfig.h>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/ImGuiBackend.hpp>

namespace {

}

/* Inspired by https://github.com/kainino0x/webgpu-cross-platform-demo */
namespace expengine {
namespace render {
namespace webgpu {

WebGpuRenderer::WebGpuRenderer(
    const std::string& appName,
    const uint32_t appVersion,
    int windowWidth,
    int windoHeight,
    EngineParameters& engineParams)
    : Renderer(engineParams)
{
    /* Window */
    /* TODO, could also fetch html template sizes with
     * "document.getElementById('canvas').width;" */
    mainWindow_ = std::make_shared<Window>(
        windowWidth,
        windoHeight,
        appName,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    /* Device */
    device_ = wgpu::Device::Acquire(emscripten_webgpu_get_device());
    device_.SetUncapturedErrorCallback(
        [](WGPUErrorType errorType, const char* message, void* pUserData) {
            auto logger = (spdlog::logger*) pUserData;
            SPDLOG_LOGGER_CALL(
                logger,
                spdlog::level::err,
                "WGPU UncapturedErrorCallback {} : {}",
                message);
        },
        nullptr);
    queue_ = device_.GetDefaultQueue();

    /* TODO main rendering context*/
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");

    /* ImGui */
#if 0
     imguiBackend_ = std::make_unique<ImguiBackend>(this, mainRenderingContext, mainWindow_);
#endif // 0
}

WebGpuRenderer::~WebGpuRenderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGPU renderer destruction");
}

void WebGpuRenderer::render()
{
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::uniform_int_distribution<int> uni(10, 17);
    std::this_thread::sleep_for(std::chrono::milliseconds(uni(rng)));
}

void WebGpuRenderer::handleEvent(const SDL_Event& event)
{ /* TODO implement */
}

void WebGpuRenderer::waitIdle()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

std::shared_ptr<Window> WebGpuRenderer::getMainWindow() { return mainWindow_; }

} // namespace webgpu
} // namespace render
} // namespace expengine
