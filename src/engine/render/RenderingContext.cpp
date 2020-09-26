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
	, logger_(spdlog::get(LOGGER_NAME))
{
	/* Create surface */
	auto [surfaceCreated, surface] = window_->createVkSurface(vkInstance);
	EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
	windowSurface_ = vk::UniqueSurfaceKHR(surface, vkInstance);

	/* Do the following once + on resize
	 * [resize-only] Destroy previous resources except swapchain
	 * Create SwapChain
	 * [resize-only] Destroy old SwapChain
	 * Create Render pass
	 * Create Image views
	 * Create Framebuffers
	 * Create Graphics pipeline (using layout + shaders)
	 * Create Sync objects + Command pools & Command buffers */

	/* Create SwapChain */
	auto [w, h] = window_->getDrawableSizeInPixels();
	vlkSwapchain_ = std::make_unique<vlk::Swapchain>(
		device, surface, vk::Extent2D { w, h });

	/* TODO Create Render pass */
	/* TODO Need swapchain surface format to define attachment */
	/* TODO receive attachments from parameters */

	/* TODO Create Image views */

	/* TODO Create Framebuffers */

	/* TODO Create Graphics pipeline  */

	/* TODO Create Sync objects + Command pools & Command buffers */
}

RenderingContext::~RenderingContext()
{
	SPDLOG_LOGGER_DEBUG(logger_, "RenderingContext destruction");
}

} // namespace render
} // namespace expengine
