#include "Window.hpp"

#include <stdexcept>

#include <SDL2/SDL_vulkan.h>

#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

Window::Window(int width, int height, const char* title)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	/* Calls SDL_Vulkan_LoadLibrary */
	sdlWindow_ = SDL_CreateWindow(
		title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
		height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	ASSERT_RESULT(sdlWindow_, "Failed to create an SDL window");
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
	const int TMP_TIMEOUT_COUNT = 100;
	static int tmpTimeout = 0;
	tmpTimeout++;

	return tmpTimeout > TMP_TIMEOUT_COUNT;
}

vk::SurfaceKHR Window::createSurface(vk::Instance vkInstance) const
{
	VkSurfaceKHR surface;
	ASSERT_RESULT(
		SDL_Vulkan_CreateSurface(sdlWindow_, vkInstance, &surface),
		"Failed to create a surface");

	return surface;
}

void Window::pollEvents()
{ /* TODO Implement */
}

void Window::waitEvents() const
{ /* TODO Implement */
}

std::vector<const char*> Window::getRequiredVkExtensions() const
{
	uint32_t extensionCount;
	ASSERT_RESULT(SDL_Vulkan_GetInstanceExtensions(
					  sdlWindow_, &extensionCount, nullptr),
				  "Failed to get the count of required Vulkan extensions "
				  "by the SDL window");

	std::vector<const char*> windowExtensions(extensionCount);
	ASSERT_RESULT(
		SDL_Vulkan_GetInstanceExtensions(sdlWindow_, &extensionCount,
										 windowExtensions.data()),
		"Failed to get the names of the Vulkan extensions required by the "
		"SDL window");

	return windowExtensions;
}

} // namespace render
} // namespace expengine