#include "VlkTools.hpp"

namespace expengine {
namespace render {
namespace vlk {

/** Based on setImageLayout by Sacha Willems & Vulkan-Samples and
modified with vulkan-hpp :
https://github.com/KhronosGroup/Vulkan-Samples/blob/master/framework/common/vk_common.cpp
*/
void transitionImageLayout(vk::CommandBuffer commandBuffer,
						   vk::Image image, vk::ImageLayout oldLayout,
						   vk::ImageLayout newLayout,
						   vk::ImageSubresourceRange subresourceRange,
						   vk::PipelineStageFlags srcMask,
						   vk::PipelineStageFlags dstMask)
{
	/* Create an image barrier object */
	vk::ImageMemoryBarrier barrier {
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = subresourceRange
	};

	/* Source layout : Source access mask controls actions that have to be
	 * finished on the source layout before it will be transitioned to the
	 * new layout */
	switch (oldLayout)
	{
	case vk::ImageLayout::eUndefined:
		/* Image layout is undefined (or does not matter). Only valid as
		 * initial layout. No flags required, listed only for
		 * completeness*/
		break;

	case vk::ImageLayout::ePreinitialized:
		/* Image is preinitialized. Only valid as initial layout for linear
		 * images, preserves memory contents. Make sure host writes have
		 * been finished */
		barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;
		break;

	case vk::ImageLayout::eColorAttachmentOptimal:
		/* Image is a color attachment. Make sure any writes to the color
		 * buffer have been finished */
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		/* Image is a depth/stencil attachment. Make sure any writes to the
		 * depth/stencil buffer have been finished */
		barrier.srcAccessMask
			= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;

	case vk::ImageLayout::eTransferSrcOptimal:
		/* Image is a transfer source. Make sure any reads from the image
		 * have been finished */
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		break;

	case vk::ImageLayout::eTransferDstOptimal:
		/* Image is a transfer destination. Make sure any writes to the
		 * image have been finished */
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::eShaderReadOnlyOptimal:
		/* Image is read by a shader. Make sure any shader reads from the
		 * image have been finished */
		barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
		break;
	default:
		/* Other source layouts aren't handled */
		break;
	}

	/* Target layout : Destination access mask controls the dependency for
	 * the new image layout */
	switch (newLayout)
	{
	case vk::ImageLayout::eTransferDstOptimal:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::eTransferSrcOptimal:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		break;

	case vk::ImageLayout::eColorAttachmentOptimal:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
		break;

	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		barrier.dstAccessMask = barrier.dstAccessMask
			| vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;

	case vk::ImageLayout::eShaderReadOnlyOptimal:
		/* Image will be read in a shader (sampler, input attachment). Make
		 * sure any writes to the image have been finished */
		if (!barrier.srcAccessMask)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite
				| vk::AccessFlagBits::eTransferWrite;
		}
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		break;
	default:
		/* Other source layouts aren't handled */
		break;
	}

	/* Put barrier inside the command buffer */
	commandBuffer.pipelineBarrier(srcMask, dstMask, {}, nullptr, nullptr,
								  barrier);
}

void transitionImageLayout(vk::CommandBuffer commandBuffer,
						   vk::Image image, vk::ImageLayout oldLayout,
						   vk::ImageLayout newLayout,
						   vk::ImageAspectFlags aspectMask,
						   vk::PipelineStageFlags srcMask,
						   vk::PipelineStageFlags dstMask)
{
	vk::ImageSubresourceRange subresourceRange { .aspectMask = aspectMask,
												 .baseMipLevel = 0,
												 .levelCount = 1,
												 .layerCount = 1 };
	transitionImageLayout(commandBuffer, image, oldLayout, newLayout,
						  subresourceRange);
}

} // namespace vlk
} // namespace render
} // namespace expengine
