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
    , renderPassStarted_(false)
    , renderPassEnded_(false)
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

    EXPENGINE_ASSERT(
        started_ && !renderPassStarted_,
        "beginRenderPass() called before calling begin() or called twice");
    commandBuffer_->beginRenderPass(info, vk::SubpassContents::eInline);
    renderPassStarted_ = true;
}

void FrameCommandBuffer::endRenderPass()
{
    EXPENGINE_ASSERT(
        renderPassStarted_ && !renderPassEnded_,
        "endRenderPass() called before beginRenderPass() or called twice");
    commandBuffer_->endRenderPass();
    renderPassEnded_ = true;
}

void FrameCommandBuffer::reset()
{
    /* TODO Reset in FrameCommandBuffer should be linked to the reset of the
     * underlying buffer */
    renderPassStarted_ = false;
    renderPassEnded_ = false;
    pushOffset_ = 0;
    bindedPipelineLayout_ = nullptr;
    CommandBuffer::reset();
}

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
