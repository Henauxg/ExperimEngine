#pragma once

#include <memory>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>

namespace expengine {

namespace render {
class Renderer;
class Window;
} // namespace render

class Engine {
public:
    Engine(const std::string& appName, const uint32_t appVersion);
    ~Engine();

    void run();

private:
    std::unique_ptr<render::Renderer> renderer_;
    std::shared_ptr<render::Window> mainWindow_;

    EngineParameters engineParams_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

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

} // namespace expengine
