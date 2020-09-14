#include "VlkBuffer.hpp"

#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace expengine {
namespace render {
namespace vlk {

Buffer::Buffer(const vk::Device device, vk::UniqueBuffer buffer,
			   vk::UniqueDeviceMemory memory, vk::DeviceSize alignment,
			   vk::DeviceSize size, vk::BufferUsageFlags usageFlags,
			   vk::MemoryPropertyFlags memoryPropertyFlags)
	: device_(device)
	, buffer_(std::move(buffer))
	, memory_(std::move(memory))
	, alignment_(alignment)
	, size_(size)
	, usageFlags_(usageFlags)
	, memoryPropertyFlags_(memoryPropertyFlags)
{
	setupDescriptor();
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
	auto mapResult = device_.mapMemory(memory_.get(), offset, size);

	mapped_ = mapResult.value;
	return mapResult.result;
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
		device_.unmapMemory(memory_.get());
		mapped_ = nullptr;
	}
}

/**
 * Attach the allocated memory block to the buffer
 *
 * @param offset (Optional) Byte offset (from the beginning) for the memory
 * region to bind
 *
 * @return vk::Result of the bindBufferMemory call
 */
vk::Result Buffer::bind(vk::DeviceSize offset)
{
	return device_.bindBufferMemory(buffer_.get(), memory_.get(), offset);
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
	memcpy(mapped_, data, size);
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
	vk::MappedMemoryRange mappedRange
		= { .memory = memory_.get(), .offset = offset, .size = size };
	return device_.flushMappedMemoryRanges(mappedRange);
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
	vk::MappedMemoryRange mappedRange
		= { .memory = memory_.get(), .offset = offset, .size = size };
	return device_.invalidateMappedMemoryRanges(mappedRange);
}

} // namespace vlk
} // namespace render
} // namespace expengine
