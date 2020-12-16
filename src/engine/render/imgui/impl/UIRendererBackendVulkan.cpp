#include "UIRendererBackendVulkan.hpp"

#include <string>

#include <engine/render/RenderingContext.hpp>
#include <engine/render/imgui/impl/ImGuiViewportPlatformData.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace {
const std::string RENDERER_BACKEND_NAME = "ExperimEngine_Vulkan_Renderer";

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate;
} pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
        Out.Color = aColor;
        Out.UV = aUV;
        gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
const uint32_t __glsl_shader_vert_spv[]
    = {0x07230203, 0x00010000, 0x00080001, 0x0000002e, 0x00000000, 0x00020011,
       0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
       0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
       0x00000004, 0x6e69616d, 0x00000000, 0x0000000b, 0x0000000f, 0x00000015,
       0x0000001b, 0x0000001c, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
       0x00000004, 0x6e69616d, 0x00000000, 0x00030005, 0x00000009, 0x00000000,
       0x00050006, 0x00000009, 0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006,
       0x00000009, 0x00000001, 0x00005655, 0x00030005, 0x0000000b, 0x0074754f,
       0x00040005, 0x0000000f, 0x6c6f4361, 0x0000726f, 0x00030005, 0x00000015,
       0x00565561, 0x00060005, 0x00000019, 0x505f6c67, 0x65567265, 0x78657472,
       0x00000000, 0x00060006, 0x00000019, 0x00000000, 0x505f6c67, 0x7469736f,
       0x006e6f69, 0x00030005, 0x0000001b, 0x00000000, 0x00040005, 0x0000001c,
       0x736f5061, 0x00000000, 0x00060005, 0x0000001e, 0x73755075, 0x6e6f4368,
       0x6e617473, 0x00000074, 0x00050006, 0x0000001e, 0x00000000, 0x61635375,
       0x0000656c, 0x00060006, 0x0000001e, 0x00000001, 0x61725475, 0x616c736e,
       0x00006574, 0x00030005, 0x00000020, 0x00006370, 0x00040047, 0x0000000b,
       0x0000001e, 0x00000000, 0x00040047, 0x0000000f, 0x0000001e, 0x00000002,
       0x00040047, 0x00000015, 0x0000001e, 0x00000001, 0x00050048, 0x00000019,
       0x00000000, 0x0000000b, 0x00000000, 0x00030047, 0x00000019, 0x00000002,
       0x00040047, 0x0000001c, 0x0000001e, 0x00000000, 0x00050048, 0x0000001e,
       0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001e, 0x00000001,
       0x00000023, 0x00000008, 0x00030047, 0x0000001e, 0x00000002, 0x00020013,
       0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006,
       0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040017,
       0x00000008, 0x00000006, 0x00000002, 0x0004001e, 0x00000009, 0x00000007,
       0x00000008, 0x00040020, 0x0000000a, 0x00000003, 0x00000009, 0x0004003b,
       0x0000000a, 0x0000000b, 0x00000003, 0x00040015, 0x0000000c, 0x00000020,
       0x00000001, 0x0004002b, 0x0000000c, 0x0000000d, 0x00000000, 0x00040020,
       0x0000000e, 0x00000001, 0x00000007, 0x0004003b, 0x0000000e, 0x0000000f,
       0x00000001, 0x00040020, 0x00000011, 0x00000003, 0x00000007, 0x0004002b,
       0x0000000c, 0x00000013, 0x00000001, 0x00040020, 0x00000014, 0x00000001,
       0x00000008, 0x0004003b, 0x00000014, 0x00000015, 0x00000001, 0x00040020,
       0x00000017, 0x00000003, 0x00000008, 0x0003001e, 0x00000019, 0x00000007,
       0x00040020, 0x0000001a, 0x00000003, 0x00000019, 0x0004003b, 0x0000001a,
       0x0000001b, 0x00000003, 0x0004003b, 0x00000014, 0x0000001c, 0x00000001,
       0x0004001e, 0x0000001e, 0x00000008, 0x00000008, 0x00040020, 0x0000001f,
       0x00000009, 0x0000001e, 0x0004003b, 0x0000001f, 0x00000020, 0x00000009,
       0x00040020, 0x00000021, 0x00000009, 0x00000008, 0x0004002b, 0x00000006,
       0x00000028, 0x00000000, 0x0004002b, 0x00000006, 0x00000029, 0x3f800000,
       0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
       0x00000005, 0x0004003d, 0x00000007, 0x00000010, 0x0000000f, 0x00050041,
       0x00000011, 0x00000012, 0x0000000b, 0x0000000d, 0x0003003e, 0x00000012,
       0x00000010, 0x0004003d, 0x00000008, 0x00000016, 0x00000015, 0x00050041,
       0x00000017, 0x00000018, 0x0000000b, 0x00000013, 0x0003003e, 0x00000018,
       0x00000016, 0x0004003d, 0x00000008, 0x0000001d, 0x0000001c, 0x00050041,
       0x00000021, 0x00000022, 0x00000020, 0x0000000d, 0x0004003d, 0x00000008,
       0x00000023, 0x00000022, 0x00050085, 0x00000008, 0x00000024, 0x0000001d,
       0x00000023, 0x00050041, 0x00000021, 0x00000025, 0x00000020, 0x00000013,
       0x0004003d, 0x00000008, 0x00000026, 0x00000025, 0x00050081, 0x00000008,
       0x00000027, 0x00000024, 0x00000026, 0x00050051, 0x00000006, 0x0000002a,
       0x00000027, 0x00000000, 0x00050051, 0x00000006, 0x0000002b, 0x00000027,
       0x00000001, 0x00070050, 0x00000007, 0x0000002c, 0x0000002a, 0x0000002b,
       0x00000028, 0x00000029, 0x00050041, 0x00000011, 0x0000002d, 0x0000001b,
       0x0000000d, 0x0003003e, 0x0000002d, 0x0000002c, 0x000100fd, 0x00010038};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
        fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
const uint32_t __glsl_shader_frag_spv[]
    = {0x07230203, 0x00010000, 0x00080001, 0x0000001e, 0x00000000, 0x00020011,
       0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
       0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0007000f, 0x00000004,
       0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000d, 0x00030010,
       0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
       0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x6c6f4366,
       0x0000726f, 0x00030005, 0x0000000b, 0x00000000, 0x00050006, 0x0000000b,
       0x00000000, 0x6f6c6f43, 0x00000072, 0x00040006, 0x0000000b, 0x00000001,
       0x00005655, 0x00030005, 0x0000000d, 0x00006e49, 0x00050005, 0x00000016,
       0x78655473, 0x65727574, 0x00000000, 0x00040047, 0x00000009, 0x0000001e,
       0x00000000, 0x00040047, 0x0000000d, 0x0000001e, 0x00000000, 0x00040047,
       0x00000016, 0x00000022, 0x00000000, 0x00040047, 0x00000016, 0x00000021,
       0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
       0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
       0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b,
       0x00000008, 0x00000009, 0x00000003, 0x00040017, 0x0000000a, 0x00000006,
       0x00000002, 0x0004001e, 0x0000000b, 0x00000007, 0x0000000a, 0x00040020,
       0x0000000c, 0x00000001, 0x0000000b, 0x0004003b, 0x0000000c, 0x0000000d,
       0x00000001, 0x00040015, 0x0000000e, 0x00000020, 0x00000001, 0x0004002b,
       0x0000000e, 0x0000000f, 0x00000000, 0x00040020, 0x00000010, 0x00000001,
       0x00000007, 0x00090019, 0x00000013, 0x00000006, 0x00000001, 0x00000000,
       0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x00000014,
       0x00000013, 0x00040020, 0x00000015, 0x00000000, 0x00000014, 0x0004003b,
       0x00000015, 0x00000016, 0x00000000, 0x0004002b, 0x0000000e, 0x00000018,
       0x00000001, 0x00040020, 0x00000019, 0x00000001, 0x0000000a, 0x00050036,
       0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005,
       0x00050041, 0x00000010, 0x00000011, 0x0000000d, 0x0000000f, 0x0004003d,
       0x00000007, 0x00000012, 0x00000011, 0x0004003d, 0x00000014, 0x00000017,
       0x00000016, 0x00050041, 0x00000019, 0x0000001a, 0x0000000d, 0x00000018,
       0x0004003d, 0x0000000a, 0x0000001b, 0x0000001a, 0x00050057, 0x00000007,
       0x0000001c, 0x00000017, 0x0000001b, 0x00050085, 0x00000007, 0x0000001d,
       0x00000012, 0x0000001c, 0x0003003e, 0x00000009, 0x0000001d, 0x000100fd,
       0x00010038};

} // namespace

namespace expengine {
namespace render {

/* Static used to give acces to vulkan device/instance to imgui static
 * callbacks. Set only once in UIRendererBackendVulkan constructor. Device
 * is destroyed after imgui backend and thus it cannot point to an invalid
 * object.
 */
static const vlk::Device* gVlkDevice = nullptr;
/* Static used to give acces to vulkan imgui rendering backend to imgui
 * static callbacks. Set only once in UIRendererBackendVulkan
 * constructor. No RenderinContext can be built by UIRendererBackendVulkan
 * after UIRendererBackendVulkan destruction, thus it cannot point to an
 * invalid object. */
static const UIRendererBackendVulkan* gUIVulkanBackend = nullptr;

/* Helper structure stored in the void* RenderUserData field of each
 * ImGuiViewport to easily retrieve rendering backend data. */
struct ImGuiViewportRendererData {
    std::shared_ptr<RenderingContext> renderingContext_;

    ImGuiViewportRendererData(std::shared_ptr<RenderingContext> renderingContext)
        : renderingContext_(renderingContext)
    {
    }
    ~ImGuiViewportRendererData() { }
};

/* Delegates */
static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
static void ImGui_ImplExpengine_RenderWindow(ImGuiViewport* viewport, void*);
static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport* viewport, void*);

UIRendererBackendVulkan::UIRendererBackendVulkan(
    std::shared_ptr<ImGuiContextWrapper> context,
    const vlk::Device& vlkDevice)
    : context_(context)
    , logger_(spdlog::get(LOGGER_NAME))
{
    /* ------------------------------------------- */
    /* Setup Renderer capabilities flags           */
    /* ------------------------------------------- */

    /* Setup back-end capabilities flags */
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = RENDERER_BACKEND_NAME.c_str();
    /* We can honor the ImDrawCmd::VtxOffset field, allowing for large
     * meshes. */
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    /* We can create multi-viewports on the Renderer side */
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    /* ------------------------------------------- */
    /* Create device objects                       */
    /* ------------------------------------------- */

    /* Font sampler */
    auto samplerResult = vlkDevice.deviceHandle().createSamplerUnique(
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
        = vlkDevice.deviceHandle().createDescriptorSetLayoutUnique(
            {.bindingCount = 1, .pBindings = binding});
    EXPENGINE_VK_ASSERT(
        descSetLayoutResult.result, "Failed to create descriptor set layout");
    descriptorSetLayout_ = std::move(descSetLayoutResult.value);

    /* Descriptor set */
    auto descriptorResult = vlkDevice.deviceHandle().allocateDescriptorSets(
        {.descriptorPool = vlkDevice.descriptorPool(),
         .descriptorSetCount = 1,
         .pSetLayouts = &descriptorSetLayout_.get()});
    EXPENGINE_VK_ASSERT(descriptorResult.result, "Failed to create descriptor set");
    descriptorSet_ = descriptorResult.value.front();

    /* Create ImGui shaders modules */
    auto vertShaderResult = vlkDevice.deviceHandle().createShaderModuleUnique(
        {.codeSize = sizeof(__glsl_shader_vert_spv),
         .pCode = (uint32_t*) __glsl_shader_vert_spv});
    EXPENGINE_VK_ASSERT(
        vertShaderResult.result, "Failed to create the ImGui vertex shader");
    vertShader_ = std::move(vertShaderResult.value);

    auto fragShaderResult = vlkDevice.deviceHandle().createShaderModuleUnique(
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
    auto pipelineLayoutResult = vlkDevice.deviceHandle().createPipelineLayoutUnique(
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
    /* Rendering bindings                          */
    /* ------------------------------------------- */

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        /* Give delegates access to vulkan device and backend */
        gVlkDevice = &vlkDevice;
        gUIVulkanBackend = this;

        /* Bind rendering delegates */
        ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
        /* No check for Platform_CreateVkSurface since handled by RC */
        platform_io.Renderer_CreateWindow = ImGui_ImplExpengine_CreateWindow;
        /* Note : also called on main viewport */
        platform_io.Renderer_DestroyWindow = ImGui_ImplExpengine_DestroyWindow;
        platform_io.Renderer_SetWindowSize = ImGui_ImplExpengine_SetWindowSize;
        platform_io.Renderer_RenderWindow = ImGui_ImplExpengine_RenderWindow;
        platform_io.Renderer_SwapBuffers = ImGui_ImplExpengine_SwapBuffers;
    }
}

UIRendererBackendVulkan::~UIRendererBackendVulkan()
{
    SPDLOG_LOGGER_DEBUG(logger_, "UIRendererBackendVulkan destruction");
    /* Clean main viewport render data if viewport is not enabled */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    if (ImGuiViewportRendererData* data
        = (ImGuiViewportRendererData*) mainViewport->RendererUserData)
        delete data;
    mainViewport->RendererUserData = nullptr;
}

vk::GraphicsPipelineCreateInfo UIRendererBackendVulkan::getPipelineInfo() const
{
    vk::GraphicsPipelineCreateInfo pipelineInfo = graphicsPipelineInfo_;
    return pipelineInfo;
};

void UIRendererBackendVulkan::uploadFonts(const vlk::Device& vlkDevice)
{
    /* Get texture data from ImGui */
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixelsBuffer;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixelsBuffer, &width, &height);
    size_t bufferSize = (size_t) width * (size_t) height * 4 * sizeof(char);

    /* Create GPU texture */
    fontTexture_ = std::make_unique<vlk::Texture>(
        vlkDevice,
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

void UIRendererBackendVulkan::bindMainRenderingContext(
    std::shared_ptr<RenderingContext> mainRenderingContext)
{
    /* Setup main viewport RendererUserData */
    /* Cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
     * enabled. Else cleaned by RendererBackend */
    ImGuiViewportRendererData* data
        = new ImGuiViewportRendererData(mainRenderingContext);
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->RendererUserData = data;
}

static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport)
{
    /* Get window from platform data */
    auto platformData = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    EXPENGINE_ASSERT(platformData != nullptr, "Error, null PlatformUserData");

    /* Get instance and device from module UIRendererBackendVulkan global
     */
    EXPENGINE_ASSERT(gVlkDevice != nullptr, "Error, null gVlkDevice");
    /* Also get a reference to the UIRendererBackendVulkan itself */
    EXPENGINE_ASSERT(gUIVulkanBackend != nullptr, "Error, null gUIVulkanBackend");

    /* Create a RenderingContext. Surface creation is handled by the
     * RC. */
    auto renderingContext = std::make_shared<RenderingContext>(
        gVlkDevice->instanceHandle(),
        *gVlkDevice,
        platformData->window_,
        *gUIVulkanBackend,
        AttachmentsFlagBits::eColorAttachment);

    /* Allocate RendererUserData */
    auto renderData = new ImGuiViewportRendererData(renderingContext);
    viewport->RendererUserData = renderData;
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
    auto renderData = (ImGuiViewportRendererData*) viewport->RendererUserData;
    if (renderData)
    {
        delete renderData;
    }
    viewport->RendererUserData = nullptr;
}

static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    /* TODO Here handle clear request : (viewport->Flags &
     * ImGuiViewportFlags_NoRendererClear) */

    /* Window size already set by platform backend. Simply notify the RC */
    renderData->renderingContext_->handleSurfaceChanges();
}

static void ImGui_ImplExpengine_RenderWindow(ImGuiViewport* viewport, void*)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    auto& cmdBuffer = renderData->renderingContext_->beginFrame();

    /* TODO Here : begin CommandBuffer & RenderPass */

    /* TODO Here : record draw commands */

    /* TODO Here : end CommandBuffer & RenderPass */
}

static void ImGui_ImplExpengine_SwapBuffers(ImGuiViewport* viewport, void*)
{
    ImGuiViewportRendererData* renderData
        = (ImGuiViewportRendererData*) viewport->RendererUserData;
    EXPENGINE_ASSERT(renderData != nullptr, "Error, null RendererUserData");

    /* Note : Queue submission for rendering is launched here instead of
     * ImGui_ImplExpengine_RenderWindow */
    renderData->renderingContext_->submitFrame();
}

} // namespace render
} // namespace expengine
