#pragma once

#include "SDL2/SDL.h"

namespace experimengine {
namespace render {

class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	bool shouldClose() const;
	void pollEvents();
	void waitEvents() const;

private:
	struct SDL_Window* sdlWindow_;
};

} // namespace render
} // namespace experimengine
