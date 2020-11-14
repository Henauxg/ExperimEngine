#pragma once

#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Texture {
public:
	/** Create texture from data buffer */
	Texture(const vlk::Device& device, void* buffer,
					 vk::DeviceSize bufferSize, vk::Format format,
					 uint32_t texWidth, uint32_t texHeight,
					 const vk::Sampler sampler,
					 vk::ImageUsageFlags imageUsageFlags
					 = vk::ImageUsageFlagBits::eSampled,
					 vk::ImageLayout targetImgLayout
					 = vk::ImageLayout::eShaderReadOnlyOptimal);

	inline vk::Image imageHandle() const { return image_.get(); };

private:
	vk::UniqueImage image_;
	vk::UniqueDeviceMemory memory_;
	vk::UniqueImageView view_;
	/* A sampler may be shared with multiples textures */
	/* TODO may create default sampler is none is provided ? */
	const vk::Sampler sampler_;
	vk::DescriptorImageInfo descriptorInfo_;
	vk::ImageLayout layout_;
	uint32_t width_ = 0;
	uint32_t height_ = 0;
	uint32_t mipLevels_ = 0;
	uint32_t layerCount_ = 0;
};
} // namespace vlk
} // namespace render
} // namespace expengine
