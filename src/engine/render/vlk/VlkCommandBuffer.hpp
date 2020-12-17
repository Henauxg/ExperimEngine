#pragma once

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device;

class CommandBuffer {
public:
    CommandBuffer(const vlk::Device& device, vk::CommandPool commandPool);

    inline const vk::CommandBuffer getHandle() const { return commandBuffer_.get(); }

    void begin(vk::CommandBufferUsageFlags flags);
    void end();
    void beginRenderPass(vk::RenderPass renderPass, vk::Framebuffer framebuffer);
    void endRenderPass();

    void copyBufferToImage(
        vk::Buffer buffer,
        vk::Image image,
        const vk::BufferImageCopy& copyRegion);

private:
    /* Handles */
    vk::CommandPool commandPool_;
    /* Owned */
    vk::UniqueCommandBuffer commandBuffer_;
};

} // namespace vlk
} // namespace render
} // namespace expengine
