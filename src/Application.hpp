#pragma once

#include <Window.hpp>
#include <Renderer.hpp>
#include <EngineParameters.hpp>
#include <memory>

namespace experimengine {
	class Application {
	public:
		void run();

	private:
		std::unique_ptr<render::Window> window;
		std::unique_ptr<render::Renderer> renderer;

		EngineParameters engineParams;

		void mainLoop();
		void renderFrame();
		void cleanup();
		void initWindow();
		void initRenderer();
	};
}