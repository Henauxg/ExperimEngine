#pragma once

#include "Window.hpp"
#include "EngineParameters.hpp"

namespace experimengine::render {
	class Renderer
	{
	public:
		Renderer(const Window& window, EngineParameters& engineParams);
		~Renderer();

		void render();
		void rendererWaitIdle();

	private:
		/**  @brief Given to the constructor. Used to create the rendering surface. */
		const Window& window;
		EngineParameters& engineParams;
	};
}