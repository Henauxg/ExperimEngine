#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/Renderer.hpp>

namespace expengine {
namespace render {
namespace wgpu {

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
    void rendererWaitIdle();
    std::shared_ptr<Window> getMainWindow();

private:
};

} // namespace wgpu
} // namespace render
} // namespace expengine
