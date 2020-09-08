#pragma once

#include <memory>

#include <engine/EngineParameters.hpp>
#include <engine/render/Renderer.hpp>
#include <engine/render/Window.hpp>

namespace expengine {
class Application {
public:
	void run();

private:
	std::unique_ptr<render::Window> window_;
	std::unique_ptr<render::Renderer> renderer_;
	EngineParameters engineParams_;

	void mainLoop();
	void renderFrame();
	void cleanup();
	void initWindow();
	void initRenderer();
};
} // namespace expengine