#pragma once

#include <EngineParameters.hpp>
#include <Renderer.hpp>
#include <Window.hpp>
#include <memory>

namespace experimengine {
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
} // namespace experimengine