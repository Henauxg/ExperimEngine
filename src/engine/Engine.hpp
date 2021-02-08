#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <SDL2/SDL_events.h>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>

namespace experim {

class Renderer;
class Window;

typedef std::function<void(float deltaT)> TickHandler;
typedef std::function<void(SDL_Event event)> EventHandler;

class Engine {
public:
    Engine(const std::string& appName, const uint32_t appVersion);
    ~Engine();

    template <class T> void onTick(T instance)
    {
        onTicks_.push_back([instance](float deltaT) { instance->onTick(deltaT); });
    };
    template <class T> void onEvent(T instance)
    {
        onEvents_.push_back(
            [instance](SDL_Event event) { instance->onEvent(event); });
    };
    void run();

private:
    /* Owned objects */
    std::unique_ptr<Renderer> renderer_;
    std::shared_ptr<Window> mainWindow_;

    EngineParameters engineParams_;
    Timer frameTimer_;
    Timer tickTimer_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

    /* User callbacks */
    std::vector<TickHandler> onTicks_;
    std::vector<EventHandler> onEvents_;

    void prepareFrame();
    void generateUI();
    void renderFrame();

    /** Executes 1 engine tick. Returns false if the engine should stop. */
    bool tick();
#ifdef __EMSCRIPTEN__
    /** Called by the JavaScript environment */
    friend void emscriptenTick(class Engine* engine);
#endif
};

} // namespace experim
