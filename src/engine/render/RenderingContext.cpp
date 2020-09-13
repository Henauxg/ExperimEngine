#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

RenderingContext::RenderingContext(const vk::Instance vkInstance,
								   const vlk::Device& device,
								   std::shared_ptr<Window> window)
	: window_(window)
	, device_(device)
	, currentFrameIndex_(0)
{
	auto [surfaceCreated, surface] = window_->createVkSurface(vkInstance);
	EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
	windowSurface_ = vk::UniqueSurfaceKHR(surface, vkInstance);
}

RenderingContext::~RenderingContext() { }

} // namespace render
} // namespace expengine
