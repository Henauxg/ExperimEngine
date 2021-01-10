#pragma once

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkCommandBuffer.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device;
class Buffer;

class FrameCommandBuffer : public CommandBuffer {
public:
    FrameCommandBuffer(
        const vlk::Device& device,
        vk::CommandPool commandPool,
        vk::RenderPass renderPass,
        vk::Framebuffer framebuffer,
        vk::Extent2D extent);

    void beginRenderPass();
    void endRenderPass();

    void bind(
        vk::Pipeline pipeline,
        vk::PipelineLayout pipelineLayout,
        vk::DescriptorSet descriptor);

    void bindBuffers(
        const Buffer& vertexBuffer,
        const Buffer& indexBuffer,
        vk::IndexType indexType);

    void setViewport(uint32_t width, uint32_t height);

    void drawIndexed(
        uint32_t indexCount,
        uint32_t firstIndex,
        int32_t vertexOffset,
        uint32_t instanceCount = 1,
        uint32_t firstInstance = 0);

    /** A PipelineLayout must have been binded to the buffer before pushing
     * *constants */
    template <typename T>
    void pushConstants(
        vk::ShaderStageFlags shaderStages,
        vk::ArrayProxy<const T> const& values)
    {
        EXPENGINE_ASSERT(
            bindedPipelineLayout_,
            "Failed to push constants : no pipeline layout binded");
        commandBuffer_->pushConstants(
            bindedPipelineLayout_, shaderStages, pushOffset_, values);
        pushOffset_ = pushOffset_ + values.size() * sizeof(T);
    }

private:
    vk::RenderPass renderPass_;
    vk::Framebuffer framebuffer_;
    vk::Extent2D extent_;

    /* Used during binding */
    vk::PipelineLayout bindedPipelineLayout_;
    /* Current offset of the push constants */
    uint32_t pushOffset_;
};

} // namespace vlk
} // namespace render
} // namespace expengine
