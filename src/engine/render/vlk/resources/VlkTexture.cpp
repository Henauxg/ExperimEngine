#include "VlkTexture.hpp"

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
	auto commandBuffer = device.createTransientCommandBuffer();

	/* Upload texData to device memory */
	auto stagingBuffer = device.createBuffer(
		bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible
			| vk::MemoryPropertyFlagBits::eHostCoherent,
		texData);

	/* TODO :
		create image
		create image view
		transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		copy stagingBuffer to image
		transition layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	*/

	device.submitTransientCommandBuffer(commandBuffer);
}

} // namespace vlk
} // namespace render
} // namespace expengine
