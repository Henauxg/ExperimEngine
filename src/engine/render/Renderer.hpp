#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/Window.hpp>

namespace spdlog {
class logger;
}

namespace expengine {

class EngineParameters;

namespace render {

/** Abstract class used to manipulate the rendering system. */
class Renderer {
public:
    virtual ~Renderer() = default;

    virtual void render() = 0;
    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void waitIdle() = 0;
    virtual std::shared_ptr<Window> getMainWindow() = 0;

protected:
    Renderer(EngineParameters& engineParams);

    EngineParameters& engineParams_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
