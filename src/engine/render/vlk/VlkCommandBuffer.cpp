#include "VlkCommandBuffer.hpp"

#include <engine/render/vlk/VlkDevice.hpp>

namespace {

} // namespace

namespace experim {
namespace vlk {

CommandBuffer::CommandBuffer(const vlk::Device& device, vk::CommandPool commandPool)
    : commandPool_(commandPool)
    , started_(false)
    , ended_(false)
{
    auto [allocResult, cmdBuffers]
        = device.deviceHandle().allocateCommandBuffersUnique(
            {.commandPool = commandPool,
             .level = vk::CommandBufferLevel::ePrimary,
             .commandBufferCount = 1});
    EXPENGINE_VK_ASSERT(allocResult, "Failed to allocate a command buffer");

    commandBuffer_ = std::move(cmdBuffers.front());
}

void CommandBuffer::reset()
{
    started_ = false;
    ended_ = false;
}

void CommandBuffer::begin(vk::CommandBufferUsageFlags flags)
{
    EXPENGINE_ASSERT(!started_, "begin() already called");
    auto beginResult = commandBuffer_->begin({.flags = flags});
    EXPENGINE_VK_ASSERT(beginResult, "Failed to begin a command buffer");
    started_ = true;
}

void CommandBuffer::end()
{
    EXPENGINE_ASSERT(
        started_ && !ended_, "End() called before calling begin() or called twice");
    auto endResult = commandBuffer_->end();
    EXPENGINE_VK_ASSERT(endResult, "Failed to end a command buffer");
    ended_ = true;
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
} // namespace experim
