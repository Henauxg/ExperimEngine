#pragma once

#include <memory>
#include <vector>

#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkSwapchain.hpp>
#include <engine/utils/Flags.hpp>

namespace expengine {
namespace render {

enum class AttachmentsFlagBits : uint32_t {
	eColorAttachment = 1,
	eDepthAttachments = 1 << 1
};
using AttachmentsFlags = Flags<AttachmentsFlagBits>;

struct FrameObjects;

class RenderingContext {
public:
	RenderingContext(const vk::Instance vkInstance,
					 const vlk::Device& device,
					 std::shared_ptr<Window> window,
					 AttachmentsFlags attachmentFlags);
	~RenderingContext();

	/* Accessors */
	inline const vk::SurfaceKHR surface() { return windowSurface_.get(); };
	inline const Window& window() const { return *window_; };

	inline vk::CommandBuffer beginSingleTimeCommand() const
	{
		return device_.createTransientCommandBuffer();
	};
	inline void endSingleTimeCommand(vk::CommandBuffer commandBuffer) const
	{
		return device_.submitTransientCommandBuffer(commandBuffer);
	};

private:
	struct FrameObjects {
		vk::UniqueCommandPool commandPool_;
		vk::UniqueCommandBuffer commandBuffer_;
		vk::UniqueFence fence_;
		// vk::Image image_;
		vk::UniqueImageView imageView_;
		vk::UniqueFramebuffer framebuffer_;
		vk::UniqueSemaphore imageAcquiredSemaphore_;
		vk::UniqueSemaphore renderCompleteSemaphore_;
	};

	const vlk::Device& device_;

	/* Owned objects */
	std::shared_ptr<const Window> window_;
	vk::UniqueSurfaceKHR windowSurface_;
	std::unique_ptr<vlk::Swapchain> vlkSwapchain_;
	vk::UniqueRenderPass renderPass_;

	/* Frames */
	uint32_t currentFrameIndex_;
	std::vector<FrameObjects> frames_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	vk::UniqueRenderPass
	createRenderPass(const vlk::Device& device,
					 const vlk::Swapchain& swapchain,
					 AttachmentsFlags attachmentsFlags);
};

} // namespace render
} // namespace expengine
