#include "WGpuRenderer.hpp"

#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5_webgpu.h>
#endif

#include <ExperimEngineConfig.h>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/ImGuiBackend.hpp>
#include <engine/render/resources/Texture.hpp>
#include <engine/render/wgpu/WGpuRenderingContext.hpp>

namespace {

}

/* Inspired by https://github.com/kainino0x/webgpu-cross-platform-demo */
namespace experim {
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
#ifdef __EMSCRIPTEN__
    device_ = wgpu::Device::Acquire(emscripten_webgpu_get_device());
#else

#endif
    device_.SetUncapturedErrorCallback(
        [](WGPUErrorType errorType, const char* message, void* pUserData) {
            auto logger = (spdlog::logger*) pUserData;
            SPDLOG_LOGGER_CALL(
                logger,
                spdlog::level::err,
                "WGPU UncapturedErrorCallback {} : {}",
                errorType,
                message);
        },
        logger_.get());
    queue_ = device_.GetDefaultQueue();

    /* Main rendering context*/
    mainRenderingContext_ = std::make_shared<WebGpuRenderingContext>(
        device_, mainWindow_, AttachmentsFlagBits::eColorAttachment);

    /* ImGui */
    imguiBackend_
        = std::make_unique<ImguiBackend>(*this, mainRenderingContext_, mainWindow_);
}

WebGpuRenderer::~WebGpuRenderer()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGPU renderer destruction");
}

bool WebGpuRenderer::handleEvent(const SDL_Event& event)
{
    bool handled = imguiBackend_->handleEvent(event);

    if (!handled)
    {
        /* TODO handle rendering events */
    }

    return handled;
}

void WebGpuRenderer::prepareFrame() { imguiBackend_->prepareFrame(); }

void WebGpuRenderer::renderFrame()
{
    mainRenderingContext_->beginFrame();
    imguiBackend_->renderFrame();
    /* TODO Main RC rendering here */
    mainRenderingContext_->submitFrame();
}

void WebGpuRenderer::waitIdle()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGPU waitIdle implementation : nothing to do");
}

std::shared_ptr<Window> WebGpuRenderer::getMainWindow() const { return mainWindow_; }

std::unique_ptr<Texture> WebGpuRenderer::createTexture() { return nullptr; }

} // namespace webgpu
} // namespace experim
