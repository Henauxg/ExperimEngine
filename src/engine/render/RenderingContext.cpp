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

	/* TODO Do the following once + on resize */

	/* Create SwapChain */
	auto [w, h] = window_->getDrawableSizeInPixels();
	vlkSwapchain_ = std::make_unique<vlk::Swapchain>(device, surface,
												  vk::Extent2D { w, h });

	/* TODO
	 * Create Render pass
	 * Create Image views
	 * Create Framebuffers
	 * Create Graphics pipeline (using layout + shaders)
	 * Create Sync objects + Command pools & Command buffers
	 */
}

RenderingContext::~RenderingContext() { }

} // namespace render
} // namespace expengine
