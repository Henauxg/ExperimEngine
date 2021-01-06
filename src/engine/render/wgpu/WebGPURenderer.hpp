#pragma once

#include <SDL2\SDL_events.h>

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu_cpp.h>
#endif

#include <engine/render/Renderer.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/ImGuiBackend.hpp>

namespace expengine {
namespace render {
namespace webgpu {

class WebGPURenderer : public Renderer {
public:
    WebGPURenderer(
        const std::string& appName,
        const uint32_t appVersion,
        int windowWidth,
        int windoHeight,
        EngineParameters& engineParams);
    ~WebGPURenderer();

    void render();
    void handleEvent(const SDL_Event& event);
    void waitIdle();
    std::shared_ptr<Window> getMainWindow();

private:
    std::shared_ptr<Window> mainWindow_;

    /* WebGPU objects */
    wgpu::Device device_;
    wgpu::Queue queue_;

    /* UI */
    std::unique_ptr<ImguiBackend> imguiBackend_;
};

} // namespace webgpu
} // namespace render
} // namespace expengine
