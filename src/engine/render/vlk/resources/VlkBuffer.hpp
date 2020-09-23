#pragma once

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device;

/**
 * Vulkan buffer wrapper.
 * Based on VulkanBuffer by Sascha Willems examples -
 * https://github.com/SaschaWillems/Vulkan */
class Buffer {
public:
	Buffer::Buffer(const vk::Device device, vk::UniqueBuffer buffer,
				   vk::UniqueDeviceMemory memory, vk::DeviceSize alignment,
				   vk::DeviceSize size, vk::BufferUsageFlags usageFlags,
				   vk::MemoryPropertyFlags memoryPropertyFlags);

	vk::Result map(vk::DeviceSize size = VK_WHOLE_SIZE,
				   vk::DeviceSize offset = 0);
	void unmap();
	vk::Result bind(vk::DeviceSize offset = 0);
	void setupDescriptor(vk::DeviceSize size = VK_WHOLE_SIZE,
						 vk::DeviceSize offset = 0);
	void copyData(void const* data, vk::DeviceSize size);
	vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE,
					 vk::DeviceSize offset = 0);
	vk::Result invalidate(vk::DeviceSize size = VK_WHOLE_SIZE,
						  vk::DeviceSize offset = 0);
	inline vk::Buffer getHandle() const { return buffer_.get(); };

private:
	const vk::Device device_;
	vk::UniqueBuffer buffer_;
	vk::UniqueDeviceMemory memory_;
	VkDescriptorBufferInfo descriptor_;
	vk::DeviceSize size_ = 0;
	vk::DeviceSize alignment_ = 0;
	void* mapped_ = nullptr;
	vk::BufferUsageFlags usageFlags_;
	vk::MemoryPropertyFlags memoryPropertyFlags_;
};
} // namespace vlk
} // namespace render
} // namespace expengine
