#include "VlkMemoryAllocator.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace expengine {
namespace render {
namespace vlk {

MemoryAllocator::MemoryAllocator(
    vk::Instance instance,
    const Device& device,
    const vk::DispatchLoaderDynamic& dispatchLoader,
    uint32_t vulkanApiVersion)
    : device_(device)
    , logger_(spdlog::get(LOGGER_NAME))
{
    VmaVulkanFunctions vulkanFunctions = {
        .vkGetPhysicalDeviceProperties
        = dispatchLoader.vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties
        = dispatchLoader.vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = dispatchLoader.vkAllocateMemory,
        .vkFreeMemory = dispatchLoader.vkFreeMemory,
        .vkMapMemory = dispatchLoader.vkMapMemory,
        .vkUnmapMemory = dispatchLoader.vkUnmapMemory,
        .vkFlushMappedMemoryRanges = dispatchLoader.vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges
        = dispatchLoader.vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = dispatchLoader.vkBindBufferMemory,
        .vkBindImageMemory = dispatchLoader.vkBindImageMemory,
        .vkGetBufferMemoryRequirements
        = dispatchLoader.vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = dispatchLoader.vkGetImageMemoryRequirements,
        .vkCreateBuffer = dispatchLoader.vkCreateBuffer,
        .vkDestroyBuffer = dispatchLoader.vkDestroyBuffer,
        .vkCreateImage = dispatchLoader.vkCreateImage,
        .vkDestroyImage = dispatchLoader.vkDestroyImage,
        .vkCmdCopyBuffer = dispatchLoader.vkCmdCopyBuffer};

    /* TODO Should also fill used extension if any */
    VmaAllocatorCreateInfo allocatorInfo {
        .physicalDevice = device.physicalHandle(),
        .device = device.deviceHandle(),
        .pVulkanFunctions = &vulkanFunctions,
        .instance = instance,
        .vulkanApiVersion = vulkanApiVersion,
    };

    vmaCreateAllocator(&allocatorInfo, &allocator_);
}

MemoryAllocator::~MemoryAllocator()
{
    SPDLOG_LOGGER_DEBUG(logger_, "MemoryAllocator destruction");
    vmaDestroyAllocator(allocator_);
}

std::unique_ptr<vlk::Buffer> MemoryAllocator::createStagingBuffer(
    vk::DeviceSize size,
    void const* dataToCopy) const
{
    auto stagingBuffer = createBuffer(
        size,
        VMA_MEMORY_USAGE_CPU_ONLY,
        vk::BufferUsageFlagBits::eTransferSrc,
        dataToCopy);
    return std::move(stagingBuffer);
}

std::unique_ptr<vlk::Buffer> MemoryAllocator::createBuffer(
    vk::DeviceSize size,
    VmaMemoryUsage memoryUsage,
    vk::BufferUsageFlags bufferUsage,
    void const* dataToCopy) const
{
    /* Create a vlk::Buffer object */

    auto buffer = std::make_unique<vlk::Buffer>(
        device_.deviceHandle(), allocator_, memoryUsage, bufferUsage, size);

    /* If available, upload dataToCopy to device */

    if (dataToCopy != nullptr)
    {
        EXPENGINE_VK_ASSERT(buffer->map(), "Failed to map memory");
        buffer->copyData(dataToCopy, size);
        /* We can flush in everycase, call will be ignored if the memory is
         * host_coherent/visilbe */
        buffer->flush();
        buffer->unmap();
    }

    return std::move(buffer);
}

} // namespace vlk
} // namespace render
} // namespace expengine
