#pragma once

#include <engine/render/vlk/VlkInclude.hpp>
#include <vma/vk_mem_alloc.h>

namespace expengine {
namespace render {
namespace vlk {

class Device;

/**
 * Vulkan buffer wrapper using VMA */
class Buffer {
public:
    Buffer(
        const vk::Device device,
        const VmaAllocator& allocator,
        VmaMemoryUsage memoryUsage,
        vk::BufferUsageFlags bufferUsage,
        vk::DeviceSize size);
    ~Buffer();

    vk::Result map(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    void unmap();
    vk::Result bind(vk::DeviceSize offset = 0);
    void setupDescriptor(
        vk::DeviceSize size = VK_WHOLE_SIZE,
        vk::DeviceSize offset = 0);
    void copyData(void const* data, vk::DeviceSize size);
    vk::Result flush(vk::DeviceSize size = VK_WHOLE_SIZE, vk::DeviceSize offset = 0);
    vk::Result invalidate(
        vk::DeviceSize size = VK_WHOLE_SIZE,
        vk::DeviceSize offset = 0);
    inline vk::Buffer getHandle() const { return buffer_.get(); };

private:
    /* Handles */
    const vk::Device device_;
    const VmaAllocator allocator_;
    VmaAllocation allocation_;

    /* Owned objects */
    vk::UniqueBuffer buffer_;

    /* Info  */
    VmaMemoryUsage memoryUsage_;
    vk::BufferUsageFlags bufferUsage_;
    vk::DeviceSize size_ = 0;
    VmaAllocationInfo allocInfo_;

    VkDescriptorBufferInfo descriptor_;
    void* mapped_ = nullptr;
};
} // namespace vlk
} // namespace render
} // namespace expengine
