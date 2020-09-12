#include "Window.hpp"

#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

Window::Window(int width, int height, const char* title, uint32_t flags)
{
	/* Calls SDL_Vulkan_LoadLibrary */
	sdlWindow_
		= SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
						   SDL_WINDOWPOS_CENTERED, width, height, flags);

	/* TODO Error handling since we will create windows on the fly
	 */
	EXPENGINE_ASSERT(sdlWindow_, "Failed to create an SDL window");
}

Window::Window(int width, int height, const char* title)
	: Window(width, height, title,
			 SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
				 | SDL_WINDOW_ALLOW_HIGHDPI)
{
}

Window::~Window()
{
	/* Calls SDL_Vulkan_UnloadLibrary*/
	SDL_DestroyWindow(sdlWindow_);
}

bool Window::shouldClose() const
{
	/* TODO Implement */
	const int TMP_TIMEOUT_COUNT = 100;
	static int tmpTimeout = 0;
	tmpTimeout++;

	return tmpTimeout > TMP_TIMEOUT_COUNT;
}

std::pair<bool, vk::SurfaceKHR>
Window::createVkSurface(vk::Instance vkInstance) const
{
	vk::SurfaceKHR surface;
	bool result = SDL_Vulkan_CreateSurface(sdlWindow_, vkInstance,
										   (VkSurfaceKHR*) &surface);

	return { result, surface };
}

bool Window::createVkSurface(vk::Instance vkInstance,
							 vk::SurfaceKHR& surfaceCreated) const
{
	bool res;
	std::tie(res, surfaceCreated) = createVkSurface(vkInstance);
	return res;
}

void Window::pollEvents()
{ /* TODO Implement */
}

void Window::waitEvents() const
{ /* TODO Implement */
}

void Window::setOpacity(float opacity)
{
	SDL_SetWindowOpacity(sdlWindow_, opacity);
}

void Window::setBordered(bool bordered)
{
	SDL_SetWindowBordered(sdlWindow_, (SDL_bool) bordered);
}

void Window::setSize(int w, int h) { SDL_SetWindowSize(sdlWindow_, w, h); }

void Window::setPosition(int x, int y)
{
	SDL_SetWindowPosition(sdlWindow_, x, y);
}

void Window::setTitle(const char* title)
{
	SDL_SetWindowTitle(sdlWindow_, title);
}

void Window::setFocus() { SDL_RaiseWindow(sdlWindow_); }

std::pair<int, int> Window::getPosition() const
{
	int x = 0, y = 0;
	SDL_GetWindowPosition(sdlWindow_, &x, &y);
	return { x, y };
}

std::pair<int, int> Window::getSize() const
{
	int w = 0, h = 0;
	SDL_GetWindowSize(sdlWindow_, &w, &h);
	return { w, h };
}

bool Window::isFocused() const
{
	return (SDL_GetWindowFlags(sdlWindow_) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool Window::isMinimized() const
{
	return (SDL_GetWindowFlags(sdlWindow_) & SDL_WINDOW_MINIMIZED) != 0;
}

void Window::hide() { SDL_HideWindow(sdlWindow_); }

void Window::show() { SDL_ShowWindow(sdlWindow_); }

uint32_t Window::getWindowId() const
{
	return SDL_GetWindowID(sdlWindow_);
}

std::vector<const char*> Window::getRequiredVkExtensions() const
{
	uint32_t extensionCount;
	/* TODO Error handling since we will create windows on the fly */
	EXPENGINE_ASSERT(
		SDL_Vulkan_GetInstanceExtensions(sdlWindow_, &extensionCount,
										 nullptr),
		"Failed to get the count of required Vulkan extensions "
		"by the SDL window");

	std::vector<const char*> windowExtensions(extensionCount);
	/* TODO Error handling since we will create windows on the fly */
	EXPENGINE_ASSERT(
		SDL_Vulkan_GetInstanceExtensions(sdlWindow_, &extensionCount,
										 windowExtensions.data()),
		"Failed to get the names of the Vulkan extensions required by the "
		"SDL window");

	return windowExtensions;
}

void* Window::getPlatformHandle() const { return (void*) sdlWindow_; }

void* Window::getPlatformHandleRaw() const
{
#if defined(_WIN32)
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWindowWMInfo(sdlWindow_, &info))
		return info.info.win.window;
#endif

	/* Else default to SDL handle. Could implement other systems later if
	 * getPlatformHandleRaw is used. */
	return (void*) sdlWindow_;
}

} // namespace render
} // namespace expengine
