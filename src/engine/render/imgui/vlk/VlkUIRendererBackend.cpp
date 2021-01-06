#include "VlkUIRendererBackend.hpp"

#include <string>

#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>
#include <engine/render/imgui/spirv/imgui_shaders_spirv.h>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkRenderer.hpp>
#include <engine/render/vlk/VlkRenderingContext.hpp>

namespace {

const std::string RENDERER_BACKEND_NAME = "ExperimEngine_Vulkan_Renderer";

} // namespace

namespace expengine {
namespace render {
namespace vlk {

VulkanUIRendererBackend::VulkanUIRendererBackend(
    std::shared_ptr<ImGuiContextWrapper> imguiContext,
    const Renderer& renderer,
    std::shared_ptr<RenderingContext> mainRenderingContext)
    : UIRendererBackend(imguiContext, mainRenderingContext, RENDERER_BACKEND_NAME)
    , renderer_(dynamic_cast<const VulkanRenderer&>(renderer))
    , device_(renderer_.getDevice())
{
    /* ------------------------------------------- */
    /* Create device objects                       */
    /* ------------------------------------------- */

    /* Font sampler */
    auto samplerResult = device_.deviceHandle().createSamplerUnique(
        {.magFilter = vk::Filter::eLinear,
         .minFilter = vk::Filter::eLinear,
         .mipmapMode = vk::SamplerMipmapMode::eLinear,
         .addressModeU = vk::SamplerAddressMode::eRepeat,
         .addressModeV = vk::SamplerAddressMode::eRepeat,
         .addressModeW = vk::SamplerAddressMode::eRepeat,
         .maxAnisotropy = 1.0f,
         .minLod = -1000,
         .maxLod = 1000});
    EXPENGINE_VK_ASSERT(samplerResult.result, "Failed to create font sampler");
    fontSampler_ = std::move(samplerResult.value);

    /* Descriptor set layout */
    vk::DescriptorSetLayoutBinding binding[1]
        = {{.descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = 1,
            .stageFlags = vk::ShaderStageFlagBits::eFragment,
            .pImmutableSamplers = &fontSampler_.get()}};
    auto descSetLayoutResult
        = device_.deviceHandle().createDescriptorSetLayoutUnique(
            {.bindingCount = 1, .pBindings = binding});
    EXPENGINE_VK_ASSERT(
        descSetLayoutResult.result, "Failed to create descriptor set layout");
    descriptorSetLayout_ = std::move(descSetLayoutResult.value);

    /* Descriptor set */
    auto descriptorResult = device_.deviceHandle().allocateDescriptorSets(
        {.descriptorPool = device_.descriptorPool(),
         .descriptorSetCount = 1,
         .pSetLayouts = &descriptorSetLayout_.get()});
    EXPENGINE_VK_ASSERT(descriptorResult.result, "Failed to create descriptor set");
    descriptorSet_ = descriptorResult.value.front();

    /* Create ImGui shaders modules */
    auto vertShaderResult = device_.deviceHandle().createShaderModuleUnique(
        {.codeSize = sizeof(__glsl_shader_vert_spv),
         .pCode = (uint32_t*) __glsl_shader_vert_spv});
    EXPENGINE_VK_ASSERT(
        vertShaderResult.result, "Failed to create the ImGui vertex shader");
    vertShader_ = std::move(vertShaderResult.value);

    auto fragShaderResult = device_.deviceHandle().createShaderModuleUnique(
        {.codeSize = sizeof(__glsl_shader_frag_spv),
         .pCode = (uint32_t*) __glsl_shader_frag_spv});
    EXPENGINE_VK_ASSERT(
        fragShaderResult.result, "Failed to create the ImGui fragment shader");
    fragShader_ = std::move(fragShaderResult.value);

    /* Shader stages */
    shaderStages_[0] = vk::PipelineShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = *vertShader_,
        .pName = "main"};
    shaderStages_[1] = vk::PipelineShaderStageCreateInfo {
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = *fragShader_,
        .pName = "main"};

    /* Pipeline layout */
    vk::PushConstantRange pushConstants[1]
        = {{.stageFlags = vk::ShaderStageFlagBits::eVertex,
            .offset = 0,
            .size = sizeof(float) * 4}};
    auto pipelineLayoutResult = device_.deviceHandle().createPipelineLayoutUnique(
        {.setLayoutCount = 1,
         .pSetLayouts = &descriptorSetLayout_.get(),
         .pushConstantRangeCount = 1,
         .pPushConstantRanges = pushConstants});
    EXPENGINE_VK_ASSERT(
        pipelineLayoutResult.result, "Failed to create pipeline layout");
    pipelineLayout_ = std::move(pipelineLayoutResult.value);

    /* Create ImGui Graphics pipeline info  */
    vertBindingDesc_
        = {.stride = sizeof(ImDrawVert), .inputRate = vk::VertexInputRate::eVertex};

    vertAttributesDesc_[0]
        = {.location = 0,
           .binding = vertBindingDesc_.binding,
           .format = vk::Format::eR32G32Sfloat,
           .offset = IM_OFFSETOF(ImDrawVert, pos)};
    vertAttributesDesc_[1]
        = {.location = 1,
           .binding = vertBindingDesc_.binding,
           .format = vk::Format::eR32G32Sfloat,
           .offset = IM_OFFSETOF(ImDrawVert, uv)};
    vertAttributesDesc_[2]
        = {.location = 2,
           .binding = vertBindingDesc_.binding,
           .format = vk::Format::eR8G8B8A8Unorm,
           .offset = IM_OFFSETOF(ImDrawVert, col)};

    vertexInfo_
        = {.vertexBindingDescriptionCount = 1,
           .pVertexBindingDescriptions = &vertBindingDesc_,
           .vertexAttributeDescriptionCount
           = static_cast<uint32_t>(vertAttributesDesc_.size()),
           .pVertexAttributeDescriptions = vertAttributesDesc_.data()};

    inputAssemblyInfo_ = {.topology = vk::PrimitiveTopology::eTriangleList};

    viewportInfo_ = vk::PipelineViewportStateCreateInfo {
        .viewportCount = 1, .scissorCount = 1};

    rasterizationInfo_
        = {.polygonMode = vk::PolygonMode::eFill,
           .cullMode = vk::CullModeFlagBits::eNone,
           .frontFace = vk::FrontFace::eCounterClockwise,
           .lineWidth = 1.0f};

    /* TODO ImGui Multisampling ? */
    multisamplingInfo_ = {.rasterizationSamples = vk::SampleCountFlagBits::e1};

    colorAttachment_
        = {.blendEnable = VK_TRUE,
           .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
           .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
           .colorBlendOp = vk::BlendOp::eAdd,
           .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
           .dstAlphaBlendFactor = vk::BlendFactor::eZero,
           .alphaBlendOp = vk::BlendOp::eAdd,
           .colorWriteMask = vk::ColorComponentFlagBits::eR
               | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB
               | vk::ColorComponentFlagBits::eA};

    depthInfo_ = vk::PipelineDepthStencilStateCreateInfo {};

    blendInfo_ = {.attachmentCount = 1, .pAttachments = &colorAttachment_};

    dynStates_ = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
    dynamicState_
        = {.dynamicStateCount = static_cast<uint32_t>(dynStates_.size()),
           .pDynamicStates = dynStates_.data()};

    graphicsPipelineInfo_
        = {.stageCount = static_cast<uint32_t>(shaderStages_.size()),
           .pStages = shaderStages_.data(),
           .pVertexInputState = &vertexInfo_,
           .pInputAssemblyState = &inputAssemblyInfo_,
           .pViewportState = &viewportInfo_,
           .pRasterizationState = &rasterizationInfo_,
           .pMultisampleState = &multisamplingInfo_,
           .pDepthStencilState = &depthInfo_,
           .pColorBlendState = &blendInfo_,
           .pDynamicState = &dynamicState_,
           .layout = pipelineLayout_.get()};
}

VulkanUIRendererBackend::~VulkanUIRendererBackend()
{
    SPDLOG_LOGGER_DEBUG(logger_, "VulkanUIRendererBackend destruction");
}

void VulkanUIRendererBackend::uploadFonts()
{
    /* Get texture data from ImGui */
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixelsBuffer;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixelsBuffer, &width, &height);
    size_t bufferSize = (size_t) width * (size_t) height * 4 * sizeof(char);

    /* Create GPU texture */
    fontTexture_ = std::make_unique<vlk::Texture>(
        device_,
        pixelsBuffer,
        bufferSize,
        vk::Format::eR8G8B8A8Unorm,
        width,
        height,
        fontSampler_.get(),
        vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
        vk::ImageLayout::eShaderReadOnlyOptimal);

    /* Store font texture identifier */
    io.Fonts->TexID = (ImTextureID)(intptr_t)(VkImage) fontTexture_->imageHandle();
}

void VulkanUIRendererBackend::renderUI(
    RenderingContext& renderingContext,
    ImDrawData* drawData) const
{
    /* Avoid rendering when minimized, scale coordinates for retina displays (screen
     * coordinates != framebuffer coordinates) */
    uint32_t fbWidth = static_cast<uint32_t>(
        drawData->DisplaySize.x * drawData->FramebufferScale.x);
    uint32_t fbHeight = static_cast<uint32_t>(
        drawData->DisplaySize.y * drawData->FramebufferScale.y);
    if (fbWidth == 0 || fbHeight == 0)
        return;

    /* TODO Upload to index and vertex buffers */

    /* need 1 index/vertex buffer pair for each frame for each viewport */
    // renderContext.requestBuffer();
    // Need to be able to reuse it if large enough, so it needs to stick between
    // frames

    /* TODO Setup render state */

    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");

    /* TODO Draw commands */
    auto& vlkRenderingContext
        = dynamic_cast<VulkanRenderingContext&>(renderingContext);
    auto& cmdBuffer = vlkRenderingContext.requestCommandBuffer();

    /* End RenderPass */
    cmdBuffer.endRenderPass();
}

} // namespace vlk
} // namespace render
} // namespace expengine
