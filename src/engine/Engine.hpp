#pragma once

#include <memory>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Renderer.hpp>
#include <engine/render/Window.hpp>

namespace expengine {

class Engine {
public:
    Engine(const std::string& appName, const uint32_t appVersion);
    ~Engine();

    void run();

private:
    std::shared_ptr<render::Window> mainWindow_;
    std::unique_ptr<render::Renderer> renderer_;
    EngineParameters engineParams_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

    void render();
};

} // namespace expengine
