#pragma once

#include "EngineParameters.hpp"
#include "Window.hpp"

namespace experimengine {
namespace render {

class Renderer {
public:
	Renderer(const Window& window, EngineParameters& engineParams);
	~Renderer();

	void render();
	void rendererWaitIdle();

private:
	/**  @brief Given to the constructor. Used to create the rendering surface. */
	const Window& window_;
	EngineParameters& engineParams_;
};

} // namespace render
} // namespace experimengine