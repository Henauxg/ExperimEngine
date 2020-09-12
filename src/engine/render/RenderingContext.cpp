#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace expengine {
namespace render {

RenderingContext::RenderingContext(const vk::Instance vkInstance,
								   std::shared_ptr<Window> window)
	: window_(window)
{
	auto [surfaceCreated, surface] = window_->createVkSurface(vkInstance);
	EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
	windowSurface_ = vk::UniqueSurfaceKHR(surface, vkInstance);
}

RenderingContext::~RenderingContext() { }

const vk::SurfaceKHR RenderingContext::getSurface()
{
	return windowSurface_.get();
}

} // namespace render
} // namespace expengine
