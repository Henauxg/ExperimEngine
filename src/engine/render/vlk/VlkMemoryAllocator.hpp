#pragma once

#include <memory>

#include <vma/vk_mem_alloc.h>

#include <engine/render/vlk/VlkInclude.hpp>

namespace spdlog {
class logger;
}

namespace expengine {
namespace render {
namespace vlk {

class Device;
class Buffer;
class Image;

class MemoryAllocator {
public:
    /**
     * @brief Create the VMA allocator
     *
     * @param vulkanApiVersion It must be a value in the format as created by macro
     * VK_MAKE_VERSION or a constant like: VK_API_VERSION_1_1, VK_API_VERSION_1_0.
     * The patch version number specified is ignored. Only the major and minor
     * versions are considered. It must be less or equal (preferably equal) to value
     * as passed to vkCreateInstance as VkApplicationInfo::apiVersion. Only
     * versions 1.0, 1.1, 1.2 are supported by the current implementation.
     */
    MemoryAllocator(
        vk::Instance instance,
        const Device& device,
        const vk::DispatchLoaderDynamic& dispatchLoader,
        uint32_t vulkanApiVersion);
    ~MemoryAllocator();

    std::unique_ptr<vlk::Buffer> createStagingBuffer(
        vk::DeviceSize size,
        void const* dataToCopy) const;

    std::unique_ptr<vlk::Buffer> createBuffer(
        vk::DeviceSize size,
        VmaMemoryUsage memoryUsage,
        vk::BufferUsageFlags bufferUsage,
        void const* dataToCopy) const;

    std::unique_ptr<vlk::Image> createTextureImage(
        vk::ImageUsageFlags imageUsageFlags,
        vk::Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels = 1,
        uint32_t layerCount = 1) const;

    std::unique_ptr<vlk::Image> createImage(
        VmaMemoryUsage memoryUsage,
        vk::ImageUsageFlags imageUsageFlags,
        vk::Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels = 1,
        uint32_t layerCount = 1) const;

private:
    /* References */
    const Device& device_;

    /* Owned objects */
    VmaAllocator allocator_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace vlk
} // namespace render
} // namespace expengine
