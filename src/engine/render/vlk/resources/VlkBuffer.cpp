#include "VlkBuffer.hpp"

#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace expengine {
namespace render {
namespace vlk {

Buffer::Buffer(
    const vk::Device device,
    const VmaAllocator& allocator,
    VmaMemoryUsage memoryUsage,
    vk::BufferUsageFlags bufferUsage,
    vk::DeviceSize size)
    : device_(device)
    , allocator_(allocator)
    , memoryUsage_(memoryUsage)
    , bufferUsage_(bufferUsage)
    , size_(size)
    , mapped_(nullptr)
    , data_(nullptr)
{
    /* Allocate and bind */

    vk::BufferCreateInfo bufferInfo {.size = size, .usage = bufferUsage};
    VmaAllocationCreateInfo allocRequestInfo = {.usage = memoryUsage};
    vk::Buffer vkBuffer;
    vmaCreateBuffer(
        allocator_,
        reinterpret_cast<VkBufferCreateInfo*>(&bufferInfo),
        &allocRequestInfo,
        reinterpret_cast<VkBuffer*>(&vkBuffer),
        &allocation_,
        &allocInfo_);

    buffer_ = vk::UniqueBuffer(vkBuffer, device);

    setupDescriptor();
}

Buffer::~Buffer()
{
    /*  Buffer is released but we need to handle the VMA allocated memory manually */
    vmaFreeMemory(allocator_, allocation_);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the
 * specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass
 * VK_WHOLE_SIZE to map the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer mapping call
 */
vk::Result Buffer::map(vk::DeviceSize size, vk::DeviceSize offset)
{
    auto res = vmaMapMemory(allocator_, allocation_, &mapped_);
    data_ = static_cast<uint8_t*>(mapped_);

    return vk::Result(res);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the
 * specified buffer range. This method asserts that the mapping succeeded
 *
 * @param size (Optional) Size of the memory range to map. Pass
 * VK_WHOLE_SIZE to map the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 */
void Buffer::assertMap(vk::DeviceSize size, vk::DeviceSize offset)
{
    auto res = vk::Result(vmaMapMemory(allocator_, allocation_, &mapped_));
    data_ = static_cast<uint8_t*>(mapped_);
    EXPENGINE_VK_ASSERT(res, "Failed to map memory for a buffer");
}

/**
 * Unmap a mapped memory range
 *
 * @note Does not return a result as unmapMemory can't fail
 */
void Buffer::unmap()
{
    if (mapped_)
    {
        vmaUnmapMemory(allocator_, allocation_);
        mapped_ = nullptr;
        data_ = nullptr;
    }
}

void Buffer::setupDescriptor(vk::DeviceSize size, vk::DeviceSize offset)
{
    descriptor_.offset = offset;
    descriptor_.buffer = *buffer_;
    descriptor_.range = size;
}

/**
 * Copies the specified data to the mapped buffer
 *
 * @param data Pointer to the data to copy
 * @param size Size of the data to copy in machine units
 *
 */
void Buffer::copyData(void const* data, vk::DeviceSize size)
{
    EXPENGINE_ASSERT(data, "Null data copied to a VlkBuffer");
    EXPENGINE_ASSERT(mapped_, "Data copied to an unmapped VlkBuffer");
    memcpy(data_, data, size);
    data_ = data_ + size;
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to flush. Pass
 * VK_WHOLE_SIZE to flush the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::Result of the flush call
 */
vk::Result Buffer::flush(vk::DeviceSize size, vk::DeviceSize offset)
{
    vmaFlushAllocation(allocator_, allocation_, offset, size);
    /* TODO No return value from vmaFlushAllocation.
     * There should be one according to the documentation
     * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/vk__mem__alloc_8h.html#a30c37c1eec6025f397be41644f48490f
     */
    return vk::Result::eSuccess;
}

void Buffer::assertFlush(vk::DeviceSize size, vk::DeviceSize offset)
{
    vmaFlushAllocation(allocator_, allocation_, offset, size);
    /* TODO No return value from vmaFlushAllocation. */
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 *
 * @param size (Optional) Size of the memory range to invalidate. Pass
 * VK_WHOLE_SIZE to invalidate the complete buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return vk::Result of the invalidate call
 */
vk::Result Buffer::invalidate(vk::DeviceSize size, vk::DeviceSize offset)
{
    vmaInvalidateAllocation(allocator_, allocation_, offset, size);

    /* TODO No return value from vmaFlushAllocation.
     * There should be one according to the documentation
     * https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/vk__mem__alloc_8h.html#a30c37c1eec6025f397be41644f48490f
     */
    return vk::Result::eSuccess;
}

void Buffer::assertInvalidate(vk::DeviceSize size, vk::DeviceSize offset)
{
    vmaInvalidateAllocation(allocator_, allocation_, offset, size);
    /* TODO No return value from vmaFlushAllocation. */
}

} // namespace vlk
} // namespace render
} // namespace expengine
