#pragma once

#include <SDL2\SDL_events.h>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>

namespace expengine {
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
    Renderer(EngineParameters& engineParams)
        : engineParams_(engineParams)
        , logger_(spdlog::get(LOGGER_NAME)) {};

    EngineParameters& engineParams_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
