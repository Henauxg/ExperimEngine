#include "VlkTexture.hpp"

#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkTools.hpp>

namespace expengine {
namespace render {
namespace vlk {

Texture::Texture(const vlk::Device& device, void* texData,
				 vk::DeviceSize bufferSize, vk::Format format,
				 uint32_t texWidth, uint32_t texHeight,
				 const vk::Sampler sampler,
				 vk::ImageUsageFlags imageUsageFlags,
				 vk::ImageLayout targetImgLayout)
	: width_(texWidth)
	, height_(texHeight)
	, sampler_(sampler)
	, mipLevels_(1)
	, layerCount_(1)
{
	/* Upload texData to accessible device memory */
	auto stagingBuffer = device.createBuffer(
		bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible
			| vk::MemoryPropertyFlagBits::eHostCoherent,
		texData);

	/* Create image (as a transfer dest) */
	vk::ImageCreateInfo imgCreateInfo
		= { .imageType = vk::ImageType::e2D,
			.format = format,
			.extent = { width_, height_, 1 },
			.mipLevels = mipLevels_,
			.arrayLayers = layerCount_,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage
			= (imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst),
			.sharingMode = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined };

	auto createImgResult
		= device.deviceHandle().createImageUnique(imgCreateInfo);
	EXPENGINE_VK_ASSERT(createImgResult.result, "Failed to create image");
	image_ = std::move(createImgResult.value);

	/* Allocate memory for the image (device local) and bind it */

	auto memRequirements
		= device.deviceHandle().getImageMemoryRequirements(image_.get());
	vk::MemoryAllocateInfo memAllocInfo {
		.allocationSize = memRequirements.size,
		.memoryTypeIndex
		= device.findMemoryType(memRequirements.memoryTypeBits,
								vk::MemoryPropertyFlagBits::eDeviceLocal)
	};
	memory_ = device.deviceHandle().allocateMemoryUnique(memAllocInfo);
	device.deviceHandle().bindImageMemory(image_.get(), memory_.get(), 0);

	auto imageCopyCmdBuffer = device.createTransientCommandBuffer();

	/* Transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL */
	vlk::transitionImageLayout(imageCopyCmdBuffer, image_.get(),
							   vk::ImageLayout::eUndefined,
							   vk::ImageLayout::eTransferDstOptimal,
							   vk::ImageAspectFlagBits::eColor);

	/* TODO : copy stagingBuffer to image */

	/* Transition layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */
	vlk::transitionImageLayout(imageCopyCmdBuffer, image_.get(),
							   vk::ImageLayout::eTransferDstOptimal,
							   vk::ImageLayout::eShaderReadOnlyOptimal,
							   vk::ImageAspectFlagBits::eColor);

	/* TODO Create Image View */

	/* TODO Implement fence signaling in buffer sumbission */
	device.submitTransientCommandBuffer(imageCopyCmdBuffer);
}

} // namespace vlk
} // namespace render
} // namespace expengine
