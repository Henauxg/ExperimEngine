#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>

namespace experim {

class Renderer;
class Window;

class Engine {
public:
    Engine(const std::string& appName, const uint32_t appVersion);
    ~Engine();

    template <class T> void onTick(T instance)
    {
        std::function<void()> a = [&]() { instance->tick(); };
        onTicks_.push_back(a);
    };
    void run();

private:
    /* Owned objects */
    std::unique_ptr<Renderer> renderer_;
    std::shared_ptr<Window> mainWindow_;

    EngineParameters engineParams_;
    Timer frameTimer;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

    /* User callbacks */
    std::vector<std::function<void()>> onTicks_;

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
