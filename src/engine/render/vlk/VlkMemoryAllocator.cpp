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
    : logger_(spdlog::get(LOGGER_NAME))
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

} // namespace vlk
} // namespace render
} // namespace expengine
