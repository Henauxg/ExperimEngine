#pragma once

#include <engine/render/imgui/UIRendererBackend.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace experim {

class Renderer;

namespace vlk {

class Device;
class VulkanRenderer;
class VlkTexture;
class FrameCommandBuffer;
struct FrameRenderBuffers;

/* Shared device objects at backend level : */
/* -> Font Texture (Image + ImageView + Memory) */
/* -> Sampler (font) */
/* -> Descriptor set layout */
/* -> Descriptor set */
/* -> Pipeline layout
/* -> Shader modules and stage info */

/** Custom back-end inspired by imgui_impl_vulkan. */
class VulkanUIRendererBackend final : public UIRendererBackend {
public:
    VulkanUIRendererBackend(
        std::shared_ptr<ImGuiContextWrapper> imguiContext,
        const Renderer& renderer,
        std::shared_ptr<RenderingContext> mainRenderingContext);
    ~VulkanUIRendererBackend();

    void uploadFonts() override;

    /** Called by ImGui callbacks for secondary viewports.
     * TODO make it fully shared between rendering backends */
    void uploadBuffersAndDraw(
        ImGuiViewportRendererData* renderData,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) override;

private:
    /* References */
    const vlk::VulkanRenderer& renderer_;
    const vlk::Device& device_;

    /* Vulkan objects shared by all the RenderingContext(s) */
    std::unique_ptr<VlkTexture> fontTexture_;
    vk::UniqueDescriptorSetLayout descriptorSetLayout_;
    /* Descriptor set is not unique since the pool owns it
     * (VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT not used)  */
    vk::DescriptorSet descriptorSet_;
    vk::UniqueSampler fontSampler_;

    /* Graphic pipeline objects. Used to create the ImGui Graphics Pipeline
     * for each RenderingContext. */
    vk::UniquePipelineLayout pipelineLayout_;
    vk::UniqueShaderModule vertShader_;
    vk::UniqueShaderModule fragShader_;
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages_;
    vk::VertexInputBindingDescription vertBindingDesc_;
    std::array<vk::VertexInputAttributeDescription, 3> vertAttributesDesc_;
    vk::PipelineVertexInputStateCreateInfo vertexInfo_;
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo_;
    vk::PipelineViewportStateCreateInfo viewportInfo_;
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo_;
    vk::PipelineMultisampleStateCreateInfo multisamplingInfo_;
    vk::PipelineColorBlendAttachmentState colorAttachment_;
    vk::PipelineDepthStencilStateCreateInfo depthInfo_;
    vk::PipelineColorBlendStateCreateInfo blendInfo_;
    std::array<vk::DynamicState, 2> dynStates_;
    vk::PipelineDynamicStateCreateInfo dynamicState_;
    /* All the above members are used to fill this
     * GraphicsPipelineCreateInfo. The only missing info in order to create
     * a Pipeline is a RenderPass which is provided by each
     * RenderingContext. */
    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo_;

    void setupRenderState(
        FrameCommandBuffer& cmdBuffer,
        const vk::Pipeline pipeline,
        FrameRenderBuffers& frame,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) const;
};

} // namespace vlk
} // namespace experim
