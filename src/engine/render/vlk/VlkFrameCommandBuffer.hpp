#pragma once

#include <engine/render/vlk/VlkCommandBuffer.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device;

class FrameCommandBuffer : public CommandBuffer {
public:
    FrameCommandBuffer(
        const vlk::Device& device,
        vk::CommandPool commandPool,
        vk::RenderPass renderPass,
        vk::Framebuffer framebuffer,
        vk::Extent2D extent);

    void endRenderPass();

private:
    /* Implicitly called in constructor for now */
    void beginRenderPass(
        vk::RenderPass renderPass,
        vk::Framebuffer framebuffer,
        vk::Extent2D extent);
};

} // namespace vlk
} // namespace render
} // namespace expengine
