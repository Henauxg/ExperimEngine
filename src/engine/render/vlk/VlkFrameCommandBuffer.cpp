#include "VlkFrameCommandBuffer.hpp"

#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/resources/VlkBuffer.hpp>

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
    , renderPass_(renderPass)
    , framebuffer_(framebuffer)
    , extent_(extent)
    , bindedPipelineLayout_(nullptr)
    , pushOffset_(0)
{
}

void FrameCommandBuffer::beginRenderPass()
{
    /* TODO here get values for clear color */
    std::array<float, 4> clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    vk::ClearValue clearColor(clearValue);
    vk::RenderPassBeginInfo info
        = {.renderPass = renderPass_,
           .framebuffer = framebuffer_,
           .renderArea = {.extent = extent_},
           .clearValueCount = 1,
           .pClearValues = &clearColor};

    /* TODO here could check for optimality of render area against renderpass
     * granularity */

    commandBuffer_->beginRenderPass(info, vk::SubpassContents::eInline);
}

void FrameCommandBuffer::endRenderPass() { commandBuffer_->endRenderPass(); }

void FrameCommandBuffer::bind(
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet descriptor)
{
    commandBuffer_->bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
    commandBuffer_->bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptor, nullptr);

    bindedPipelineLayout_ = pipelineLayout;
    pushOffset_ = 0;
}

void FrameCommandBuffer::bindBuffers(
    const Buffer& vertexBuffer,
    const Buffer& indexBuffer,
    vk::IndexType indexType)
{
    uint32_t vertexOffset = 0;
    commandBuffer_->bindVertexBuffers(0, vertexBuffer.getHandle(), vertexOffset);
    commandBuffer_->bindIndexBuffer(indexBuffer.getHandle(), 0, indexType);
}

void FrameCommandBuffer::setViewport(uint32_t width, uint32_t height)
{
    commandBuffer_->setViewport(
        0,
        vk::Viewport {
            .x = 0,
            .y = 0,
            .width = static_cast<float>(width),
            .height = static_cast<float>(height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f});
}

void FrameCommandBuffer::drawIndexed(
    uint32_t indexCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t instanceCount,
    uint32_t firstInstance)
{
    commandBuffer_->drawIndexed(
        indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

} // namespace vlk
} // namespace render
} // namespace expengine
