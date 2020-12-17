#include "VlkFrameCommandBuffer.hpp"

#include <engine/render/vlk/VlkDevice.hpp>

namespace {

} // namespace

namespace expengine {
namespace render {
namespace vlk {

FrameCommandBuffer::FrameCommandBuffer(
    const vlk::Device& device,
    vk::CommandPool commandPool,
    vk::RenderPass renderPass,
    vk::Framebuffer framebuffer,
    vk::Extent2D extent)
    : CommandBuffer(device, commandPool)
{
    beginRenderPass(renderPass, framebuffer, extent);
}

void FrameCommandBuffer::beginRenderPass(
    vk::RenderPass renderPass,
    vk::Framebuffer framebuffer,
    vk::Extent2D extent)
{
    /* TODO here set values for clear color */
    vk::RenderPassBeginInfo info
        = {.renderPass = renderPass,
           .framebuffer = framebuffer,
           .renderArea = {.extent = extent}};

    commandBuffer_->beginRenderPass(info, vk::SubpassContents::eInline);
}

void FrameCommandBuffer::endRenderPass() { commandBuffer_->endRenderPass(); }

} // namespace vlk
} // namespace render
} // namespace expengine
