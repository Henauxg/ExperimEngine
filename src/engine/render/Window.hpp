#pragma once

#include <vector>

#include <SDL2/SDL.h>

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {

class Window {
public:
	Window(int width, int height, const char* title);
	~Window();

	bool shouldClose() const;
	void pollEvents();
	void waitEvents() const;
	void setOpacity(float opacity);
	void setBordered(bool bordered);
	void setSize(int w, int h);
	void setPosition(int x, int y);
	void hide();
	void show();

	std::vector<const char*> Window::getRequiredVkExtensions() const;
	vk::SurfaceKHR createSurface(vk::Instance vkInstance) const;

private:
	struct SDL_Window* sdlWindow_;
};

} // namespace render
} // namespace expengine
