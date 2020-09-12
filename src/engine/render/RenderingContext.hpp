#pragma once

#include <memory>

#include <engine/render/Window.hpp>

namespace expengine {
namespace render {

class RenderingContext {
public:
	RenderingContext(const vk::Instance vkInstance,
					 std::shared_ptr<Window> window);
	~RenderingContext();

	const vk::SurfaceKHR getSurface();

	inline const Window& window() const { return *window_; };

private:
	std::shared_ptr<const Window> window_;
	vk::UniqueSurfaceKHR windowSurface_;
	/* TODO VkSurfaceFormatKHR */
};

} // namespace render
} // namespace expengine
