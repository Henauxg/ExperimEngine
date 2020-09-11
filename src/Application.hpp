#pragma once

#include <memory>

#include <engine/EngineParameters.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Renderer.hpp>
#include <engine/render/Window.hpp>

namespace expengine {

class Application {
public:
	Application::Application();
	void run();

private:
	std::shared_ptr<render::Window> window_;
	std::unique_ptr<render::Renderer> renderer_;
	EngineParameters engineParams_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	void mainLoop();
	void renderFrame();
	void cleanup();
};

} // namespace expengine
