#pragma once

#include <engine/render/vlk/VlkInclude.hpp>
#include <vma/vk_mem_alloc.h>

namespace expengine {
namespace render {
namespace vlk {

class Device;

/**
 * Vulkan Image wrapper using VMA */
class Image {
public:
    Image(
        const vk::Device device,
        const VmaAllocator& allocator,
        VmaMemoryUsage memoryUsage,
        vk::ImageUsageFlags imageUsageFlags,
        vk::Format format,
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels = 1,
        uint32_t layerCount = 1);
    ~Image();

    inline const vk::Image getHandle() { return image_.get(); };
    inline const vk::Extent3D getExtent() { return imgInfo_.extent; };
    inline const vk::ImageLayout getLayout() { return layout_; }

    /**
     * @brief Put an image memory barrier for setting an image layout on the
     * sub resource into the given command buffer
     */
    void transitionImageLayout(
        vk::CommandBuffer commandBuffer,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::ImageSubresourceRange subresourceRange,
        vk::PipelineStageFlags srcMask = vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlags dstMask = vk::PipelineStageFlagBits::eAllCommands);

    /**
     * @brief Put an image memory barrier for setting an image layout into the
     * given command buffer. Fixed ImageSubresourceRange on first mip level and
     * layer
     */
    void transitionImageLayout(
        vk::CommandBuffer commandBuffer,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::ImageAspectFlags aspectMask,
        vk::PipelineStageFlags srcMask = vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlags dstMask = vk::PipelineStageFlagBits::eAllCommands);

private:
    /* Handles */
    const vk::Device device_;
    const VmaAllocator allocator_;
    VmaAllocation allocation_;

    /* Owned objects */
    vk::UniqueImage image_;

    /* Info  */
    vk::ImageCreateInfo imgInfo_;
    VmaMemoryUsage memoryUsage_;
    VmaAllocationInfo allocInfo_;
    vk::ImageLayout layout_;
};

} // namespace vlk
} // namespace render
} // namespace expengine
