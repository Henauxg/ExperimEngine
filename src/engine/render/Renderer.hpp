#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/IRendering.hpp>
#include <engine/render/Window.hpp>

namespace spdlog {
class logger;
}

namespace experim {

struct EngineParameters;

/** Abstract class used to manipulate the rendering system. */
class Renderer : public IRendering {
public:
    virtual ~Renderer() = default;

    virtual bool handleEvent(const SDL_Event& event) = 0;
    virtual void prepareFrame() = 0;
    virtual void renderFrame() = 0;

    virtual void waitIdle() = 0;
    virtual std::shared_ptr<Window> getMainWindow() const = 0;

protected:
    Renderer(EngineParameters& engineParams);

    EngineParameters& engineParams_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace experim
