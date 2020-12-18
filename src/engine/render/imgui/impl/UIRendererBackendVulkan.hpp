#pragma once

#include <engine/render/Window.hpp>
#include <engine/render/imgui/impl/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/lib/imgui.h>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkFrameCommandBuffer.hpp>
#include <engine/render/vlk/resources/VlkTexture.hpp>

namespace expengine {
namespace render {

class RenderingContext;

/* Shared device objects at backend level : */
/* -> Font Texture (Image + ImageView + Memory) */
/* -> Sampler (font) */
/* -> Descriptor set layout */
/* -> Descriptor set */
/* -> Pipeline layout
/* -> Shader modules and stage info */

/** Custom back-end inspired by imgui_impl_vulkan. */
class UIRendererBackendVulkan {
public:
    UIRendererBackendVulkan(
        std::shared_ptr<ImGuiContextWrapper> context,
        const vlk::Device& vlkDevice);
    ~UIRendererBackendVulkan();

    void uploadFonts(const vlk::Device& vlkDevice);

    /* This function should be called after the constructor and before
     * rendering. Provides ImGui with access to the main RenderingContext
     * from the main viewport. */
    void bindMainRenderingContext(
        std::shared_ptr<RenderingContext> mainRenderingContext);

    /* Returns a modifiable copy of the backend prepared
     * vk::GraphicsPipelineCreateInfo */
    vk::GraphicsPipelineCreateInfo getPipelineInfo() const;

    /* Called by ImGui callbacks for secondary viewports */
    void renderUI(ImDrawData* draw_data, vlk::FrameCommandBuffer& commandBuffer)
        const;

private:
    /* ImGui */
    const std::shared_ptr<ImGuiContextWrapper> context_;

    /* Vulkan objects shared by all the RenderingContext(s) */
    std::unique_ptr<vlk::Texture> fontTexture_;
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

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
