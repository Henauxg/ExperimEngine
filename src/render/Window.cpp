#include "Window.hpp"

#include <stdexcept>
//#include <SDL2/SDL_vulkan.h>

namespace experimengine {
namespace render {

Window::Window(int width, int height, const char* title)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	/* Calls SDL_Vulkan_LoadLibrary */
	sdlWindow_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
								  height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (sdlWindow_ == NULL) {
		throw std::runtime_error("Failed to create a window");
	}
}

Window::~Window()
{
	/* Calls SDL_Vulkan_UnloadLibrary*/
	SDL_DestroyWindow(sdlWindow_);
	SDL_Quit();
}

bool Window::shouldClose() const
{
	/* TODO Implement */
	return false;
}

void Window::pollEvents()
{ /* TODO Implement */
}

void Window::waitEvents() const
{ /* TODO Implement */
}

} // namespace render
} // namespace experimengine