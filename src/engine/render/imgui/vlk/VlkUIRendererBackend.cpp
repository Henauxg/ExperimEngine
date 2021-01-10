#include "VlkUIRendererBackend.hpp"

#include <string>

#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>
#include <engine/render/imgui/spirv/imgui_shaders_spirv.h>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkFrameCommandBuffer.hpp>
#include <engine/render/vlk/VlkRenderer.hpp>
#include <engine/render/vlk/VlkRenderingContext.hpp>
#include <engine/render/vlk/resources/VlkTexture.hpp>

namespace {

const std::string RENDERER_BACKEND_NAME = "ExperimEngine_Vulkan_Renderer";

} // namespace

namespace expengine {
namespace render {
namespace vlk {

struct FrameRenderBuffers {
    std::unique_ptr<Buffer> vertexBuffer = nullptr;
    std::unique_ptr<Buffer> indexBuffer = nullptr;
};

/** The Vulkan-specific derived class  stored in the void*
 * RenderUserData field of each ImGuiViewport */
class VkImGuiViewportRendererData : public ImGuiViewportRendererData {
public:
    ImGuiViewportRendererData* clone(
        std::shared_ptr<RenderingContext> renderingContext) override
    {
        /* This will be stored by ImGui in a (void *).
         * Will be cleaned by ImGui_ImplExpengine_DestroyWindow */
        return new VkImGuiViewportRendererData(
            renderingContext, graphicsPipelineInfo_);
    };

    /** Constructor used publicly only once for the main viewport. Other viewports
     * will clone the main one. */
    VkImGuiViewportRendererData(
        std::shared_ptr<RenderingContext> renderingContext,
        vk::GraphicsPipelineCreateInfo& graphicsPipelineInfo)
        : ImGuiViewportRendererData(renderingContext)
        , graphicsPipelineInfo_(graphicsPipelineInfo)
    {
        /* Initialize viewport objects */
        onSurfaceChange();
    }

    FrameRenderBuffers& requestFrameRenderBuffers()
    {
        return renderBuffers_.at(frameIndex_);
        frameIndex_ = (frameIndex_ + 1) % renderBuffers_.size();
    }

    const vk::Pipeline pipeline() { return *uiGraphicsPipeline_; }

protected:
    /* Owned objects */
    vk::UniquePipeline uiGraphicsPipeline_;
    std::vector<FrameRenderBuffers> renderBuffers_;
    uint32_t frameIndex_;

    /* Configuration */
    /* Hold a copy since it will be modified. */
    vk::GraphicsPipelineCreateInfo graphicsPipelineInfo_;

    void onSurfaceChange() override
    {
        auto vkRenderingContext
            = std::dynamic_pointer_cast<VulkanRenderingContext>(renderingContext_);

        /* (Re)build Graphics pipeline */
        uiGraphicsPipeline_
            = vkRenderingContext->createGraphicsPipeline(graphicsPipelineInfo_);

        /* (Re)build buffers */
        frameIndex_ = 0;
        renderBuffers_.resize(vkRenderingContext->imageCount());
    };
};

VulkanUIRendererBackend::VulkanUIRendererBackend(
    std::shared_ptr<ImGuiContextWrapper> imguiContext,
    const Renderer& renderer,
    std::shared_ptr<RenderingContext> mainRenderingContext)
    : UIRendererBackend(imguiContext, RENDERER_BACKEND_NAME)
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

    /* ------------------------------------------- */
    /*  Setup main viewport RendererUserData       */
    /* ------------------------------------------- */

    /* Cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
     * enabled. Else cleaned by RendererBackend */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->RendererUserData = new VkImGuiViewportRendererData(
        mainRenderingContext, graphicsPipelineInfo_);
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
    fontTexture_ = std::make_unique<Texture>(
        device_,
        pixelsBuffer,
        bufferSize,
        vk::Format::eR8G8B8A8Unorm,
        width,
        height,
        fontSampler_.get());

    /* Store font texture identifier */
    io.Fonts->TexID = (ImTextureID)(intptr_t)(VkImage) fontTexture_->imageHandle();
}

void VulkanUIRendererBackend::renderUI(
    ImGuiViewportRendererData* rendererData,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight) const
{
    auto vlkViewportData = dynamic_cast<VkImGuiViewportRendererData*>(rendererData);
    auto& vlkRenderingContext
        = dynamic_cast<VulkanRenderingContext&>(*rendererData->renderingContext_);

    /* ------------------
     * Upload to index and vertex buffers
     *------------------ */

    auto& frame = vlkViewportData->requestFrameRenderBuffers();

    if (drawData->TotalVtxCount > 0)
    {
        size_t vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        if (frame.vertexBuffer == nullptr || frame.vertexBuffer->size() < vertexSize)
            frame.vertexBuffer = device_.allocator().createVertexBuffer(vertexSize);
        if (frame.indexBuffer == nullptr || frame.indexBuffer->size() < indexSize)
            frame.indexBuffer = device_.allocator().createIndexBuffer(indexSize);

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];
            frame.vertexBuffer->copyData(
                cmdList->VtxBuffer.Data,
                cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            frame.indexBuffer->copyData(
                cmdList->IdxBuffer.Data,
                cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
        }
        /* TODO Could flush both allocation at once */
        frame.vertexBuffer->assertFlush();
        frame.indexBuffer->assertFlush();

        frame.vertexBuffer->unmap();
        frame.indexBuffer->unmap();
    }

    auto& cmdBuffer = vlkRenderingContext.requestCommandBuffer();
    cmdBuffer.beginRenderPass();

    /* ------------------
     * Setup render state
     *------------------ */

    setupRenderState(
        cmdBuffer, vlkViewportData->pipeline(), frame, drawData, fbWidth, fbHeight);

    /* ------------------
     * Draw commands
     *------------------ */

    /* Will project scissor/clipping rectangles into framebuffer space */
    ImVec2 clipOff = drawData->DisplayPos; // (0,0) unless using multi-viewports
    ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina
                                                   // display which are often (2,2)

    /* Because all buffers are merged into a single one, we maintain an offset into
     * them */
    uint32_t globalVertexOffset = 0;
    uint32_t globalIndexOffset = 0;
    for (uint32_t n = 0; n < (uint32_t) drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[n];
        for (uint32_t cmd_i = 0; cmd_i < (uint32_t) cmdList->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                /* User callback, registered via ImDrawList::AddCallback()
                 * (ImDrawCallback_ResetRenderState is a special callback value used
                 * by the user to request the renderer to reset render state.) */
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    setupRenderState(
                        cmdBuffer,
                        vlkViewportData->pipeline(),
                        frame,
                        drawData,
                        fbWidth,
                        fbHeight);
                else
                    pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                /* Project scissor/clipping rectangles into framebuffer space */
                ImVec4 clipRect {
                    (pcmd->ClipRect.x - clipOff.x) * clipScale.x,
                    (pcmd->ClipRect.y - clipOff.y) * clipScale.y,
                    (pcmd->ClipRect.z - clipOff.x) * clipScale.x,
                    (pcmd->ClipRect.w - clipOff.y) * clipScale.y};

                if (clipRect.x < fbWidth && clipRect.y < fbHeight
                    && clipRect.z >= 0.0f && clipRect.w >= 0.0f)
                {
                    /* Negative offsets are illegal for vkCmdSetScissor */
                    if (clipRect.x < 0.0f)
                        clipRect.x = 0.0f;
                    if (clipRect.y < 0.0f)
                        clipRect.y = 0.0f;

                    /* Apply scissor/clipping rectangle */
                    vk::Rect2D scissor {
                        .offset
                        = {.x = (int32_t)(clipRect.x), .y = (int32_t)(clipRect.y)},
                        .extent
                        = {.width = (uint32_t)(clipRect.z - clipRect.x),
                           .height = (uint32_t)(clipRect.w - clipRect.y)}};
                    cmdBuffer.getHandle().setScissor(0, scissor);

                    /* Draw */
                    cmdBuffer.drawIndexed(
                        pcmd->ElemCount,
                        pcmd->IdxOffset + globalIndexOffset,
                        pcmd->VtxOffset + globalVertexOffset);
                }
            }
        }
        globalVertexOffset += cmdList->IdxBuffer.Size;
        globalIndexOffset += cmdList->VtxBuffer.Size;
    }

    /* End RenderPass */
    cmdBuffer.endRenderPass();
}

void VulkanUIRendererBackend::setupRenderState(
    FrameCommandBuffer& cmdBuffer,
    const vk::Pipeline pipeline,
    FrameRenderBuffers& frame,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight) const
{
    /* Bind objects to command buffer */
    cmdBuffer.bind(pipeline, *pipelineLayout_, descriptorSet_);
    cmdBuffer.bindBuffers(
        *frame.vertexBuffer,
        *frame.indexBuffer,
        sizeof(ImDrawIdx) == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);

    /* Viewport */
    cmdBuffer.setViewport(fbWidth, fbHeight);

    /* Setup scale and translation:
     * The visible imgui space lies from draw_data->DisplayPos (top left) to
     * draw_data->DisplayPos + data_data->DisplaySize (bottom right). DisplayPos is
     * (0,0) for single viewport apps. */
    std::array<float, 2> scale
        = {2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y};
    cmdBuffer.pushConstants<float>(vk::ShaderStageFlagBits::eVertex, scale);

    std::array<float, 2> translate
        = {-1.0f - drawData->DisplayPos.x * scale[0],
           -1.0f - drawData->DisplayPos.y * scale[1]};
    cmdBuffer.pushConstants<float>(vk::ShaderStageFlagBits::eVertex, translate);
}

} // namespace vlk
} // namespace render
} // namespace expengine
