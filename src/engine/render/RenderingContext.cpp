#include "RenderingContext.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

/* Vulkan objects per RenderingContext :
 * All the objects except the surface are recreated on resize.
 * -> Surface
 * -> SwapChain
 * -> 1 Render pass shared by UI and application
 * -> 2x Graphics pipeline (1x for ImGui, 1x for the application rendering)
 * -> Per Image (image count Backbuffers)
 * --> Command pool
 * --> Command buffer
 * --> Fence
 * --> Semaphores (x2)
 * --> Image view  (BackbufferView)
 * --> Framebuffer */

RenderingContext::RenderingContext(
	const vk::Instance vkInstance, const vlk::Device& device,
	std::shared_ptr<Window> window,
	const UIRendererBackendVulkan& imguiRenderBackend,
	AttachmentsFlags attachmentsFlags)
	: window_(window)
	, device_(device)
	, imguiRenderBackend_(imguiRenderBackend)
	, attachmentsFlags_(attachmentsFlags)
	, frameIndex_(0)
	, semaphoreIndex_(0)
	, logger_(spdlog::get(LOGGER_NAME))
{
	/* Create surface */
	auto [surfaceCreated, surface] = window_->createVkSurface(vkInstance);
	EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
	windowSurface_ = vk::UniqueSurfaceKHR(surface, vkInstance);

	initOrResizeRenderingContext();
}

RenderingContext::~RenderingContext()
{
	SPDLOG_LOGGER_DEBUG(logger_, "RenderingContext destruction");
}

void RenderingContext::initOrResizeRenderingContext()
{
	/* Create SwapChain with images */
	auto [w, h] = window_->getDrawableSizeInPixels();
	vlkSwapchain_ = std::make_unique<vlk::Swapchain>(
		device_, *windowSurface_, vk::Extent2D { w, h });

	/* Create Render pass */
	renderPass_
		= createRenderPass(device_, *vlkSwapchain_, attachmentsFlags_);

	/* Create Graphics pipeline(s)  */
	auto uiPipelineInfo = imguiRenderBackend_.getPipelineInfo();
	uiGraphicsPipeline_
		= createGraphicsPipeline(device_, uiPipelineInfo, *renderPass_);

	/* Create frame objects : Image views, Framebuffers, Command pools,
	 * Command buffers and Sync objects */
	createFrameObjects(frames_, *vlkSwapchain_, *renderPass_,
					   attachmentsFlags_);
}

void RenderingContext::createFrameObjects(
	std::vector<FrameObjects>& frames, const vlk::Swapchain& swapchain,
	vk::RenderPass& renderPass, AttachmentsFlags attachmentsFlags)
{
	/* Destroy the previous frame objects */
	frames.clear();
	semaphores_.clear();
	semaphoreToFrameFence_.clear();
	frameIndex_ = 0;
	semaphoreIndex_ = 0;

	/* Configuration shared by all the frames */

	/* Image view */
	vk::ImageViewCreateInfo imageViewInfo
		= { .viewType = vk::ImageViewType::e2D,
			.format = swapchain.getSurfaceFormat().format,
			.components = { .r = vk::ComponentSwizzle::eR,
							.g = vk::ComponentSwizzle::eG,
							.b = vk::ComponentSwizzle::eB,
							.a = vk::ComponentSwizzle::eA },
			.subresourceRange
			= { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

	/* Framebuffer */
	auto imageExtent = swapchain.getImageExtent();
	std::array<vk::ImageView, 2> attachments;
	vk::FramebufferCreateInfo framebufferInfo
		= { .renderPass = renderPass,
			.pAttachments = attachments.data(),
			.width = imageExtent.width,
			.height = imageExtent.height,
			.layers = 1 };

	if (attachmentsFlags & AttachmentsFlagBits::eColorAttachment)
		framebufferInfo.attachmentCount++;
	if (attachmentsFlags & AttachmentsFlagBits::eDepthAttachments)
	{
		/* TODO attachments[framebufferInfo.attachmentCount] =
		 * depthimageView; */
		framebufferInfo.attachmentCount++;
	}

	/* Create 1 frame object for each swapchain image */
	for (const auto& image : swapchain.getImages())
	{
		/* Create the image view */
		imageViewInfo.image = image;

		auto [imageViewResult, imageView]
			= device_.deviceHandle().createImageViewUnique(imageViewInfo);
		EXPENGINE_VK_ASSERT(imageViewResult,
							"Failed to create an image view");

		/* Create the framebuffer */
		if (attachmentsFlags & AttachmentsFlagBits::eColorAttachment)
			attachments[0] = imageView.get();

		auto [framebufferResult, framebuffer]
			= device_.deviceHandle().createFramebufferUnique(
				framebufferInfo);
		EXPENGINE_VK_ASSERT(framebufferResult,
							"Failed to create a framebuffer");

		/* Create the command pool */
		auto [cmdPoolResult, commandPool]
			= device_.deviceHandle().createCommandPoolUnique(
				{ .flags
				  = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
				  .queueFamilyIndex
				  = device_.queueIndices().graphicsFamily.value() });
		EXPENGINE_VK_ASSERT(framebufferResult,
							"Failed to create a command pool");

		/* Create the command buffer */
		auto [cmdBufferResult, cmdBuffer]
			= device_.deviceHandle().allocateCommandBuffersUnique(
				{ .commandPool = *commandPool,
				  .level = vk::CommandBufferLevel::ePrimary,
				  .commandBufferCount = 1 });
		EXPENGINE_VK_ASSERT(cmdBufferResult,
							"Failed to create a command buffer");

		/* Create the fence */
		auto [fenceResult, fence]
			= device_.deviceHandle().createFenceUnique(
				{ .flags = vk::FenceCreateFlagBits::eSignaled });
		EXPENGINE_VK_ASSERT(fenceResult, "Failed to create a fence");

		/* Create the Frame object */
		FrameObjects frame;
		frame.imageView_ = std::move(imageView);
		frame.framebuffer_ = std::move(framebuffer);
		frame.commandPool_ = std::move(commandPool);
		frame.commandBuffer_ = std::move(cmdBuffer.front());
		frame.fence_ = std::move(fence);
		frames_.push_back(std::move(frame));

		/* Create the semaphores */
		auto imageAcqSemaphore
			= device_.deviceHandle().createSemaphoreUnique({});
		EXPENGINE_VK_ASSERT(
			imageAcqSemaphore.result,
			"Failed to create the imageAcquired Semaphore");
		auto renderCompleteSemaphore
			= device_.deviceHandle().createSemaphoreUnique({});
		EXPENGINE_VK_ASSERT(
			renderCompleteSemaphore.result,
			"Failed to create the renderComplete Semaphore");

		/* Create a Frame Semaphore object */
		FrameSemaphores semaphoreObject;
		semaphoreObject.imageAcquired_
			= std::move(imageAcqSemaphore.value);
		semaphoreObject.renderComplete_
			= std::move(renderCompleteSemaphore.value);
		semaphores_.push_back(std::move(semaphoreObject));
	}
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

vk::UniquePipeline RenderingContext::createGraphicsPipeline(
	const vlk::Device& device,
	vk::GraphicsPipelineCreateInfo& pipelineInfos,
	vk::RenderPass& renderPass)
{
	pipelineInfos.renderPass = renderPass;

	auto [result, graphicsPipeline]
		= device.deviceHandle().createGraphicsPipelineUnique(
			nullptr, pipelineInfos);
	EXPENGINE_VK_ASSERT(result, "Failed to create a graphics pipeline");

	return std::move(graphicsPipeline);
}

void RenderingContext::handleSurfaceChanges()
{
	// TODO check swapchain and call initOrResizeRenderingContext if
	// needed.
}

vk::CommandBuffer& RenderingContext::beginFrame()
{
	/* Here we check for the semaphore availability. If semaphoreIndex_ is
	 * still used by a frame not yet submitted, we wait for its frame to be
	 * fully submitted. */
	auto fenceIt = semaphoreToFrameFence_.find(semaphoreIndex_);
	if (fenceIt != semaphoreToFrameFence_.end())
	{
		device_.deviceHandle().waitForFences(fenceIt->second, VK_TRUE, 0);
	}

	/* Acquire an image from the swapchain */
	vk::Semaphore& imgAcqSemaphore
		= semaphores_[semaphoreIndex_].imageAcquired_.get();
	auto nextImage = vlkSwapchain_->acquireNextImage(imgAcqSemaphore);

	/* Handle swapchain result : may recreate Context Objects */
	if (nextImage.result == vk::Result::eSuboptimalKHR
		|| nextImage.result == vk::Result::eErrorOutOfDateKHR)
	{
		handleSurfaceChanges();
		nextImage = vlkSwapchain_->acquireNextImage(imgAcqSemaphore);
	}
	EXPENGINE_VK_ASSERT(nextImage.result,
						"Error, could not acquire Swapchain image");
	frameIndex_ = nextImage.value;
	auto& frame = frames_.at(frameIndex_);

	/* Wait on frame fence. If this frame is already in use, we wait for it
	 * to be fully submitted before reusing it. vk:queueSubmit will signal
	 * the fence when the frame can be reused. */
	/* We could check if the fence for this frame is the same as the frame
	 * we waited on for the semaphore above. But waitForFences should
	 * return immediatly anyway if the fence is in the signaled state.*/
	device_.deviceHandle().waitForFences(frame.fence_.get(), VK_TRUE, 0);

	/* Link (through its fence) the used semaphore ID to the Frame Object
	 * given by the SwapChain . */
	semaphoreToFrameFence_.insert_or_assign(semaphoreIndex_,
											frame.fence_.get());

	/* Reset command pool/buffers */
	device_.deviceHandle().resetCommandPool(frame.commandPool_.get(), {});

	/* Return command buffer for this frame */
	return frames_.at(frameIndex_).commandBuffer_.get();
}

void RenderingContext::submitFrame(const vk::CommandBuffer& cmdBuffer)
{
	// TODO Reset frame fence
	// TODO Submit buffer(s) to queue with frame fence

	// TODO Present frame
	// TODO Handle result : may recreate swapchain
}
} // namespace render
} // namespace expengine
