#pragma once

#include <vector>

#include <engine/render/vlk/VlkInclude.hpp>

struct SDL_Window;

namespace expengine {
namespace render {

class Window {
public:
	Window(int width, int height, const char* title);
	Window(int width, int height, const char* title, uint32_t flags);
	~Window();

	bool shouldClose() const;
	void pollEvents();
	void waitEvents() const;
	void setOpacity(float opacity);
	void setBordered(bool bordered);
	void setSize(int w, int h);
	void setPosition(int x, int y);
	void setTitle(const char* title);
	void setFocus();
	std::pair<int, int> getPosition() const;
	std::pair<int, int> getSize() const;
	bool isFocused();
	bool isMinimized();
	void hide();
	void show();
	uint32_t getWindowId() const;
	std::vector<const char*> Window::getRequiredVkExtensions() const;

	/* Platform/OS specific */
	void* getPlatformHandle();
	void* getPlatformHandleRaw();

	/* Vulkan only */
	std::pair<bool, vk::SurfaceKHR>
	createVkSurface(vk::Instance vkInstance) const;
	bool createVkSurface(vk::Instance vkInstance,
						 vk::SurfaceKHR& surfaceCreated) const;

private:
	struct SDL_Window* sdlWindow_;
};

} // namespace render
} // namespace expengine
