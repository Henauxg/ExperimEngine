#include "WGpuUIRendererBackend.hpp"

#include <string>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>
#include <engine/render/imgui/wgpu/spirv/wgpu_imgui_shaders_spirv.h>
#include <engine/render/wgpu/WGpuRenderer.hpp>

namespace {

const std::string RENDERER_BACKEND_NAME = "ExperimEngine_WebGPU_Renderer";

/* No viewports for Emscripten backend */
#ifdef __EMSCRIPTEN__
const bool BACKEND_HAS_VIEWPORTS = false;
#else
const bool BACKEND_HAS_VIEWPORTS = true;
#endif

} // namespace

namespace expengine {
namespace render {
namespace webgpu {

WebGpuUIRendererBackend::WebGpuUIRendererBackend(
    std::shared_ptr<ImGuiContextWrapper> imguiContext,
    const Renderer& renderer,
    std::shared_ptr<RenderingContext> mainRenderingContext)
    : UIRendererBackend(
        imguiContext,
        RENDERER_BACKEND_NAME,
        true,
        BACKEND_HAS_VIEWPORTS)
    , renderer_(dynamic_cast<const WebGpuRenderer&>(renderer))
    , device_(renderer_.device())
{
    /* ------------------------------------------- */
    /* Create device objects                       */
    /* ------------------------------------------- */

    /* Font sampler */
    {
        wgpu::SamplerDescriptor samplerDescriptor = {
            .addressModeU = wgpu::AddressMode::Repeat,
            .addressModeV = wgpu::AddressMode::Repeat,
            .addressModeW = wgpu::AddressMode::Repeat,
            .magFilter = wgpu::FilterMode::Linear,
            .minFilter = wgpu::FilterMode::Linear,
            .mipmapFilter = wgpu::FilterMode::Linear,
        };
        fontSampler_ = device_.CreateSampler(&samplerDescriptor);
    }

    /* Bind groups */
    std::array<wgpu::BindGroupLayoutEntry, 2> commonBindGroupLayoutEntries
        = {wgpu::BindGroupLayoutEntry {
               .binding = 0,
               .visibility = wgpu::ShaderStage::Vertex,
               .type = wgpu::BindingType::UniformBuffer},
           wgpu::BindGroupLayoutEntry {
               .binding = 1,
               .visibility = wgpu::ShaderStage::Fragment,
               .type = wgpu::BindingType::Sampler}};

    wgpu::BindGroupLayoutEntry textureBindGroupLayoutEntry
        = {.binding = 0,
           .visibility = wgpu::ShaderStage::Fragment,
           .type = wgpu::BindingType::SampledTexture};

    wgpu::BindGroupLayoutDescriptor commonBindGroupLayoutDesc
        = {.entryCount = commonBindGroupLayoutEntries.size(),
           .entries = commonBindGroupLayoutEntries.data()};

    wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDesc
        = {.entryCount = 1, .entries = &textureBindGroupLayoutEntry};

    std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts
        = {device_.CreateBindGroupLayout(&commonBindGroupLayoutDesc),
           device_.CreateBindGroupLayout(&textureBindGroupLayoutDesc)};

    /* Create ImGui shaders modules */
    wgpu::ShaderModule vertexShaderModule {};
    {
        wgpu::ShaderModuleSPIRVDescriptor spirvDesc {};
        spirvDesc.codeSize = sizeof(__glsl_wgpu_shader_vert_spv);
        spirvDesc.code = (uint32_t*) __glsl_wgpu_shader_vert_spv;

        wgpu::ShaderModuleDescriptor descriptor {.nextInChain = &spirvDesc};
        vertexShaderModule = device_.CreateShaderModule(&descriptor);
    }
    wgpu::ProgrammableStageDescriptor vertexShaderDescriptor {
        .module = vertexShaderModule, .entryPoint = "main"};

    wgpu::ShaderModule fragmentShaderModule {};
    {
        wgpu::ShaderModuleSPIRVDescriptor spirvDesc {};
        spirvDesc.codeSize = sizeof(__glsl_wgpu_shader_vert_spv);
        spirvDesc.code = (uint32_t*) __glsl_wgpu_shader_vert_spv;

        wgpu::ShaderModuleDescriptor descriptor {.nextInChain = &spirvDesc};
        fragmentShaderModule = device_.CreateShaderModule(&descriptor);
    }
    wgpu::ProgrammableStageDescriptor fragmentShaderDescriptor {
        .module = fragmentShaderModule, .entryPoint = "main"};

    /* Pipeline layout */
    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc {
        .bindGroupLayoutCount = bindGroupLayouts.size(),
        .bindGroupLayouts = bindGroupLayouts.data()};

    wgpu::PipelineLayout pipelineLayout
        = device_.CreatePipelineLayout(&pipelineLayoutDesc);

    /* Create ImGui Graphics pipeline info  */
    std::array<wgpu::VertexAttributeDescriptor, 3> vertAttributesDesc;
    vertAttributesDesc[0]
        = {wgpu::VertexFormat::Float2, (uint64_t) IM_OFFSETOF(ImDrawVert, pos), 0};
    vertAttributesDesc[1]
        = {wgpu::VertexFormat::Float2, (uint64_t) IM_OFFSETOF(ImDrawVert, uv), 1};
    vertAttributesDesc[2] = {
        wgpu::VertexFormat::UChar4Norm, (uint64_t) IM_OFFSETOF(ImDrawVert, col), 2};

    wgpu::VertexBufferLayoutDescriptor vertBufferLayoutDesc
        = {.arrayStride = sizeof(ImDrawVert),
           .stepMode = wgpu::InputStepMode::Vertex,
           .attributeCount = vertAttributesDesc.size(),
           .attributes = vertAttributesDesc.data()};

    /* indexFormat : See issue https://github.com/gpuweb/gpuweb/issues/767 */
    wgpu::VertexStateDescriptor vertexStateDesc
        = {.indexFormat = wgpu::IndexFormat::Undefined,
           .vertexBufferCount = 1,
           .vertexBuffers = &vertBufferLayoutDesc};

    wgpu::RasterizationStateDescriptor rasterizationInfo
        = {.frontFace = wgpu::FrontFace::CW,
           .cullMode = wgpu::CullMode::None,
           .depthBias = 0,
           .depthBiasSlopeScale = 0,
           .depthBiasClamp = 0};

    wgpu::ColorStateDescriptor colorState
        /* TODO : get the format from the swapchain ? */
        = {.format = wgpu::TextureFormat::RGBA8Unorm,
           .writeMask = wgpu::ColorWriteMask::All};
    colorState.alphaBlend
        = {.operation = wgpu::BlendOperation::Add,
           .srcFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
           .dstFactor = wgpu::BlendFactor::Zero};
    colorState.colorBlend
        = {.operation = wgpu::BlendOperation::Add,
           .srcFactor = wgpu::BlendFactor::SrcAlpha,
           .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha};

    wgpu::RenderPipelineDescriptor graphicsPipelineInfo
        = {.layout = pipelineLayout,
           .vertexStage = vertexShaderDescriptor,
           .fragmentStage = &fragmentShaderDescriptor,
           .vertexState = &vertexStateDesc,
           .primitiveTopology = wgpu::PrimitiveTopology::TriangleList,
           .rasterizationState = &rasterizationInfo,
           .sampleCount = 1,
           .colorStateCount = 1,
           .colorStates = &colorState,
           .sampleMask = std::numeric_limits<uint32_t>::max(),
           .alphaToCoverageEnabled = false};
    graphicsPipeline_ = device_.CreateRenderPipeline(&graphicsPipelineInfo);

    /* ------------------------------------------- */
    /*  Setup main viewport RendererUserData       */
    /* ------------------------------------------- */
    /* Cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
     * enabled. Else cleaned by RendererBackend */
    /* TODO */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

WebGpuUIRendererBackend::~WebGpuUIRendererBackend()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuUIRendererBackend destruction");
}

void WebGpuUIRendererBackend::uploadFonts()
{
    /* Get texture data from ImGui */
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixelsBuffer;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixelsBuffer, &width, &height);
    size_t bufferSize = (size_t) width * (size_t) height * 4 * sizeof(char);

    /* TODO Create GPU texture */

    /* TODO Store font texture identifier */

    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

void WebGpuUIRendererBackend::uploadBuffersAndDraw(
    ImGuiViewportRendererData* renderData,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight) const
{
    /* TODO Upload to index and vertex buffers */

    /* TODO Setup render state */

    /* TODO Draw commands */

    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

} // namespace webgpu
} // namespace render
} // namespace expengine
