#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

RenderingContext::RenderingContext(const vk::Instance vkInstance,
								   const vlk::Device& device,
								   std::shared_ptr<Window> window,
								   AttachmentsFlags attachmentsFlags)
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

	/* Create SwapChain with images */
	auto [w, h] = window_->getDrawableSizeInPixels();
	vlkSwapchain_ = std::make_unique<vlk::Swapchain>(
		device, surface, vk::Extent2D { w, h });

	/* Create Render pass */
	renderPass_
		= createRenderPass(device, *vlkSwapchain_, attachmentsFlags);

	/* Create frame objects : Image views */
	frames_.clear();

	vk::ImageViewCreateInfo imageViewInfo
		= { .viewType = vk::ImageViewType::e2D,
			.format = vlkSwapchain_->getSurfaceFormat().format,
			.components = { .r = vk::ComponentSwizzle::eR,
							.g = vk::ComponentSwizzle::eG,
							.b = vk::ComponentSwizzle::eB,
							.a = vk::ComponentSwizzle::eA },
			.subresourceRange
			= { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };
	for (const auto& image : vlkSwapchain_->getImages())
	{
		imageViewInfo.image = image;

		auto [result, imageView]
			= device_.deviceHandle().createImageViewUnique(imageViewInfo);
		EXPENGINE_VK_ASSERT(result, "Failed to retrieve swapchain images");

		FrameObjects frame;
		frame.imageView_ = std::move(imageView);
		frames_.push_back(std::move(frame));
	}

	/* Create frame objects : Framebuffers */
	vk::FramebufferCreateInfo framebufferInfo
		= { .renderPass = *renderPass_, .layers = 1 };

	std::array<vk::ImageView, 2> attachments;
	if (attachmentsFlags & AttachmentsFlagBits::eColorAttachment)
		framebufferInfo.attachmentCount++;
	if (attachmentsFlags & AttachmentsFlagBits::eDepthAttachments)
	{
		/* TODO attachments[framebufferInfo.attachmentCount] =
		 * depthimageView; */
		framebufferInfo.attachmentCount++;
	}

	for (auto& frame : frames_)
	{
		if (attachmentsFlags & AttachmentsFlagBits::eColorAttachment)
			attachments[0] = frame.imageView_.get();

		auto [result, framebuffer]
			= device_.deviceHandle().createFramebufferUnique(
				framebufferInfo);
		EXPENGINE_VK_ASSERT(result, "Failed to create framebuffers");

		frame.framebuffer_ = std::move(framebuffer);
	}

	/* TODO Create Graphics pipeline  */

	/* TODO Create Sync objects + Command pools & Command buffers */
}

RenderingContext::~RenderingContext()
{
	SPDLOG_LOGGER_DEBUG(logger_, "RenderingContext destruction");
}

vk::UniqueRenderPass
RenderingContext::createRenderPass(const vlk::Device& device,
								   const vlk::Swapchain& swapchain,
								   AttachmentsFlags attachmentsFlags)
{
	/* Init subpass */
	vk::SubpassDescription subpass
		= { .pipelineBindPoint = vk::PipelineBindPoint::eGraphics };
	vk::SubpassDependency dependency {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
	};

	/* TODO : May rework attachments configuration */
	std::vector<vk::AttachmentDescription> attachments;

	/* Color attachment */
	vk::AttachmentReference colorAttachmentRef;
	if (attachmentsFlags & AttachmentsFlagBits::eColorAttachment)
	{
		vk::AttachmentDescription colorAttachment {
			.format = swapchain.getSurfaceFormat().format,
			.samples = vk::SampleCountFlagBits::e1,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout = vk::ImageLayout::eUndefined,
			.finalLayout = vk::ImageLayout::ePresentSrcKHR
		};
		colorAttachmentRef
			= { .attachment = static_cast<uint32_t>(attachments.size()),
				.layout = vk::ImageLayout::eColorAttachmentOptimal };
		attachments.push_back(colorAttachment);

		/* Update the subpass */
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		dependency.srcStageMask
			|= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstStageMask
			|= vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask
			|= vk::AccessFlagBits::eColorAttachmentWrite;
	}

	/* Depth attachment */
	vk::AttachmentReference depthAttachmentRef;
	if (attachmentsFlags & AttachmentsFlagBits::eDepthAttachments)
	{
		vk::AttachmentDescription depthAttachment {
			.format = device_.getDepthFormat(),
			.samples = vk::SampleCountFlagBits::e1,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout = vk::ImageLayout::eUndefined,
			.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
		};
		depthAttachmentRef
			= { .attachment = static_cast<uint32_t>(attachments.size()),
				.layout
				= vk::ImageLayout::eDepthStencilAttachmentOptimal };
		attachments.push_back(depthAttachment);

		/* Update the subpass */
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		dependency.srcStageMask
			|= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstStageMask
			|= vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstAccessMask
			|= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}

	auto [result, renderPass]
		= device.deviceHandle().createRenderPassUnique(
			{ .attachmentCount = static_cast<uint32_t>(attachments.size()),
			  .pAttachments = attachments.data(),
			  .subpassCount = 1,
			  .pSubpasses = &subpass,
			  .dependencyCount = 1,
			  .pDependencies = &dependency });
	EXPENGINE_VK_ASSERT(result, "Failed to create a render pass");

	return std::move(renderPass);
}

} // namespace render
} // namespace expengine
