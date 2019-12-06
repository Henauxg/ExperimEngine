#pragma once

#include "SDL2/SDL.h"
#include "vlk/VlkInclude.hpp"
#include <vector>

namespace expengine {
namespace render {

class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	bool shouldClose() const;
	void pollEvents();
	void waitEvents() const;

	std::vector<const char*> Window::getRequiredVkExtensions() const;
	vk::SurfaceKHR createSurface(vk::Instance vkInstance) const;

private:
	struct SDL_Window* sdlWindow_;
};

} // namespace render
} // namespace expengine
