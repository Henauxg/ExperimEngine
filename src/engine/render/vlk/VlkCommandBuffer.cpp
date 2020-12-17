#include "VlkCommandBuffer.hpp"

#include <engine/render/vlk/VlkDevice.hpp>

namespace {

} // namespace

namespace expengine {
namespace render {
namespace vlk {

CommandBuffer::CommandBuffer(const vlk::Device& device, vk::CommandPool commandPool)
    : commandPool_(commandPool)
{
    auto [allocResult, cmdBuffers]
        = device.deviceHandle().allocateCommandBuffersUnique(
            {.commandPool = commandPool,
             .level = vk::CommandBufferLevel::ePrimary,
             .commandBufferCount = 1});
    EXPENGINE_VK_ASSERT(allocResult, "Failed to allocate a command buffer");

    commandBuffer_ = std::move(cmdBuffers.front());
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags flags)
{
    auto beginResult = commandBuffer_->begin({.flags = flags});
    EXPENGINE_VK_ASSERT(beginResult, "Failed to begin a command buffer");
}

void CommandBuffer::end()
{
    auto endResult = commandBuffer_->end();
    EXPENGINE_VK_ASSERT(endResult, "Failed to end a command buffer");
}

void CommandBuffer::copyBufferToImage(
    vk::Buffer buffer,
    vk::Image image,
    const vk::BufferImageCopy& copyRegion)
{
    commandBuffer_->copyBufferToImage(
        buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion);
}

} // namespace vlk
} // namespace render
} // namespace expengine
