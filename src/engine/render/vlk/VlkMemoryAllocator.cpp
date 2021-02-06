#include "VlkMemoryAllocator.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/resources/VlkBuffer.hpp>
#include <engine/render/vlk/resources/VlkImage.hpp>

namespace experim {
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

/**
 * Creates an index buffer.
 *
 * @note The buffer is guaranteed to be host_visible
 *
 * @param size Size of the buffer to create
 * @param offset (Optional) Data to be copied into the buffer, if data is present,
 * the buffer will be mapped/filled/unmapped/flushed automatically.
 *
 * @return unique_ptr to the vlk::Buffer
 */
std::unique_ptr<vlk::Buffer> MemoryAllocator::createIndexBuffer(
    vk::DeviceSize size,
    void const* dataToCopy) const
{
    auto indexBuffer = createBuffer(
        size,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        vk::BufferUsageFlagBits::eIndexBuffer,
        dataToCopy);

    return std::move(indexBuffer);
}

/**
 * Creates a vertex buffer.
 *
 * @note The buffer is guaranteed to be host_visible
 *
 * @param size Size of the buffer to create
 * @param offset (Optional) Data to be copied into the buffer, if data is present,
 * the buffer will be mapped/filled/unmapped/flushed automatically.
 *
 * @return unique_ptr to the vlk::Buffer
 */
std::unique_ptr<vlk::Buffer> MemoryAllocator::createVertexBuffer(
    vk::DeviceSize size,
    void const* dataToCopy) const
{
    auto vertexBuffer = createBuffer(
        size,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        vk::BufferUsageFlagBits::eVertexBuffer,
        dataToCopy);

    return std::move(vertexBuffer);
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
        buffer->assertMap();
        buffer->copyData(dataToCopy, size);
        /* We can flush in everycase, call will be ignored if the memory is
         * host_coherent/visible */
        buffer->assertFlush();
        buffer->unmap();
    }

    return std::move(buffer);
}

std::unique_ptr<vlk::Image> MemoryAllocator::createTextureImage(
    vk::ImageUsageFlags imageUsageFlags,
    vk::Format format,
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    uint32_t layerCount) const
{
    /* Create a vlk::Image object */
    auto image = createImage(
        VMA_MEMORY_USAGE_GPU_ONLY,
        imageUsageFlags,
        format,
        width,
        height,
        mipLevels,
        layerCount);

    return std::move(image);
}

std::unique_ptr<vlk::Image> MemoryAllocator::createImage(
    VmaMemoryUsage memoryUsage,
    vk::ImageUsageFlags imageUsageFlags,
    vk::Format format,
    uint32_t width,
    uint32_t height,
    uint32_t mipLevels,
    uint32_t layerCount) const
{
    /* Create a vlk::Image object */
    auto image = std::make_unique<vlk::Image>(
        device_.deviceHandle(),
        allocator_,
        memoryUsage,
        imageUsageFlags,
        format,
        width,
        height,
        mipLevels,
        layerCount);

    return std::move(image);
}

} // namespace vlk
} // namespace experim
