#pragma once

#include <engine/render/vlk/VlkInclude.hpp>

namespace experim {
namespace vlk {

class Device;

class CommandBuffer {
public:
    CommandBuffer(const vlk::Device& device, vk::CommandPool commandPool);

    inline const vk::CommandBuffer getHandle() const { return commandBuffer_.get(); }

    virtual void reset();

    void begin(vk::CommandBufferUsageFlags flags);
    void end();

    void copyBufferToImage(
        vk::Buffer buffer,
        vk::Image image,
        const vk::BufferImageCopy& copyRegion);

protected:
    /* Handles */
    vk::CommandPool commandPool_;
    /* Owned */
    vk::UniqueCommandBuffer commandBuffer_;

    bool started_;
    bool ended_;
};

} // namespace vlk
} // namespace experim
