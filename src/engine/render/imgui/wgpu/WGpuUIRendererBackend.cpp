#include "WGpuUIRendererBackend.hpp"

#include <string>

#include <glm/glm.hpp>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>
#include <engine/render/imgui/wgpu/spirv/wgpu_imgui_shaders_spirv.h>
#include <engine/render/wgpu/WGpuRenderer.hpp>
#include <engine/render/wgpu/WGpuRenderingContext.hpp>
#include <engine/render/wgpu/resources/WGpuTexture.hpp>

namespace {

const std::string RENDERER_BACKEND_NAME = "ExperimEngine_WebGPU_Renderer";

/* No viewports for Emscripten backend */
#ifdef __EMSCRIPTEN__
const bool BACKEND_HAS_VIEWPORTS = false;
#else
const bool BACKEND_HAS_VIEWPORTS = true;
#endif

/* TODO : Found no way to retrieve an image count from the swapchain. Is there such a
 * thing in WebGPU ?
 * Should there be any sync in order to reuse buffers (index/vertex) ?
 */
const uint32_t FAKE_SWAPCHAIN_IMAGE_COUNT = 3;

} // namespace

/* TODO : Custom texture handling */
/* TODO : Vertex/Index buffers encapsulation */
/* TODO : Error handling for WebGPU objects */

namespace expengine {
namespace render {
namespace webgpu {

struct FrameRenderBuffers {
    wgpu::Buffer vertexBuffer = nullptr;
    uint32_t vertexBufferSize = 0;
    wgpu::Buffer indexBuffer = nullptr;
    uint32_t indexBufferSize = 0;
};

/** The backend-specific derived class stored in the void*
 * RenderUserData field of each ImGuiViewport */
class WGpuImGuiViewportRendererData final : public ImGuiViewportRendererData {
public:
    ImGuiViewportRendererData* clone(
        std::shared_ptr<RenderingContext> renderingContext) override
    {
        /* This will be stored by ImGui in a (void *).
         * Will be cleaned by ImGui_ImplExpengine_DestroyWindow */
        return new WGpuImGuiViewportRendererData(renderingContext);
    };

    /** Constructor used publicly only once for the main viewport. Other viewports
     * will clone the main one. */
    WGpuImGuiViewportRendererData(std::shared_ptr<RenderingContext> renderingContext)
        : ImGuiViewportRendererData(renderingContext)
    {
        /* Initialize viewport objects */
        onSurfaceChange();
    }

    ~WGpuImGuiViewportRendererData()
    {
        SPDLOG_DEBUG("WGpuImGuiViewportRendererData destruction");
    }

    FrameRenderBuffers& requestFrameRenderBuffers()
    {
        return renderBuffers_.at(frameIndex_);
        frameIndex_ = (frameIndex_ + 1) % renderBuffers_.size();
    }

protected:
    /* Owned objects */
    std::vector<FrameRenderBuffers> renderBuffers_;
    uint32_t frameIndex_;

    void onSurfaceChange() override
    {
        auto wgpuRenderingContext
            = std::dynamic_pointer_cast<WebGpuRenderingContext>(renderingContext_);

        /* Reset buffers container */
        frameIndex_ = 0;
        renderBuffers_.resize(FAKE_SWAPCHAIN_IMAGE_COUNT);
    }
};

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
        wgpu::SamplerDescriptor samplerDescriptor {
            .label = "WebGpuUIRendererBackend font sampler",
            .addressModeU = wgpu::AddressMode::Repeat,
            .addressModeV = wgpu::AddressMode::Repeat,
            .addressModeW = wgpu::AddressMode::Repeat,
            .magFilter = wgpu::FilterMode::Linear,
            .minFilter = wgpu::FilterMode::Linear,
            .mipmapFilter = wgpu::FilterMode::Linear,
        };
        fontSampler_ = device_.CreateSampler(&samplerDescriptor);
    }

    /* Bind groups layouts */
    std::array<wgpu::BindGroupLayoutEntry, 2> commonBindGroupLayoutEntries {
        wgpu::BindGroupLayoutEntry {
            .binding = 0,
            .visibility = wgpu::ShaderStage::Vertex,
            .type = wgpu::BindingType::UniformBuffer},
        wgpu::BindGroupLayoutEntry {
            .binding = 1,
            .visibility = wgpu::ShaderStage::Fragment,
            .type = wgpu::BindingType::Sampler}};

    wgpu::BindGroupLayoutEntry textureBindGroupLayoutEntry {
        .binding = 0,
        .visibility = wgpu::ShaderStage::Fragment,
        .type = wgpu::BindingType::SampledTexture};

    wgpu::BindGroupLayoutDescriptor commonBindGroupLayoutDesc {
        .entryCount = commonBindGroupLayoutEntries.size(),
        .entries = commonBindGroupLayoutEntries.data()};

    wgpu::BindGroupLayoutDescriptor textureBindGroupLayoutDesc {
        .entryCount = 1, .entries = &textureBindGroupLayoutEntry};

    std::array<wgpu::BindGroupLayout, 2> bindGroupLayouts {
        device_.CreateBindGroupLayout(&commonBindGroupLayoutDesc),
        device_.CreateBindGroupLayout(&textureBindGroupLayoutDesc)};

    imageBindGroupLayout_ = bindGroupLayouts[1];

    /* Create ImGui shaders modules */
    wgpu::ShaderModule vertexShaderModule {};

    wgpu::ShaderModuleSPIRVDescriptor vertSpirvDesc {};
    vertSpirvDesc.codeSize = sizeof(__glsl_wgpu_shader_vert_spv) / sizeof(uint32_t);
    vertSpirvDesc.code = (uint32_t*) __glsl_wgpu_shader_vert_spv;

    wgpu::ShaderModuleDescriptor vertDescriptor {.nextInChain = &vertSpirvDesc};
    vertexShaderModule = device_.CreateShaderModule(&vertDescriptor);

    wgpu::ProgrammableStageDescriptor vertexShaderDescriptor {
        .module = vertexShaderModule, .entryPoint = "main"};

    wgpu::ShaderModule fragmentShaderModule {};

    wgpu::ShaderModuleSPIRVDescriptor fragSpirvDesc {};
    fragSpirvDesc.codeSize = sizeof(__glsl_wgpu_shader_frag_spv) / sizeof(uint32_t);
    fragSpirvDesc.code = (uint32_t*) __glsl_wgpu_shader_frag_spv;

    wgpu::ShaderModuleDescriptor fragDescriptor {.nextInChain = &fragSpirvDesc};
    fragmentShaderModule = device_.CreateShaderModule(&fragDescriptor);

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

    /* Create the rendering pipeline */
    wgpu::RenderPipelineDescriptor graphicsPipelineInfo
        = {.label = "WebGpuUIRendererBackend rendering pipeline",
           .layout = pipelineLayout,
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

    /* Create the uniform buffer for scale and translation */
    wgpu::BufferDescriptor uniformBuffDesc {
        .label = "WebGpuUIRendererBackend uniform buffer",
        .usage = wgpu::BufferUsage::Uniform,
        .size = sizeof(glm::mat4)};
    uniformBuffer_ = device_.CreateBuffer(&uniformBuffDesc);

    /* Bind groups creation */
    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries {
        wgpu::BindGroupEntry {
            .binding = 0, .buffer = uniformBuffer_, .size = sizeof(glm::mat4)},
        wgpu::BindGroupEntry {.binding = 1, .sampler = fontSampler_}};

    wgpu::BindGroupDescriptor commonBindGroupDesc {
        .layout = bindGroupLayouts[0],
        .entryCount = bindGroupEntries.size(),
        .entries = bindGroupEntries.data()};
    commonBindGroup_ = device_.CreateBindGroup(&commonBindGroupDesc);

    /* ------------------------------------------- */
    /*  Setup main viewport RendererUserData       */
    /* ------------------------------------------- */

    /* Cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
     * enabled. Else cleaned by RendererBackend */
    ImGuiViewport* mainViewport = ImGui::GetMainViewport();
    mainViewport->RendererUserData
        = new WGpuImGuiViewportRendererData(mainRenderingContext);
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

    /* Create GPU texture */
    fontTexture_ = std::make_unique<Texture>(
        device_,
        pixelsBuffer,
        bufferSize,
        wgpu::TextureFormat::RGBA8Unorm,
        width,
        height);

    /* Store font texture identifier */
    io.Fonts->TexID = (ImTextureID)(intptr_t) fontTexture_->viewHandle().Get();

    /* Register the font bind group */
    /* TODO Add safety*/
    wgpu::BindGroupEntry imageBindGroupEntry {
        .binding = 0, .textureView = fontTexture_->viewHandle()};
    wgpu::BindGroupDescriptor imageBindGroupDesc {
        .layout = imageBindGroupLayout_,
        .entryCount = 1,
        .entries = &imageBindGroupEntry};
    imageBindGroup_ = device_.CreateBindGroup(&imageBindGroupDesc);
}

void WebGpuUIRendererBackend::uploadBuffersAndDraw(
    ImGuiViewportRendererData* rendererData,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight) const
{
    auto wgpuViewportData
        = dynamic_cast<WGpuImGuiViewportRendererData*>(rendererData);
    auto& wgpuRenderingContext
        = dynamic_cast<WebGpuRenderingContext&>(*rendererData->renderingContext_);

    /* ------------------
     * Upload to index and vertex buffers
     *------------------ */

    auto& frame = wgpuViewportData->requestFrameRenderBuffers();

    if (drawData->TotalVtxCount > 0)
    {
        size_t vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);

        if (frame.vertexBufferSize == 0 || frame.vertexBufferSize < vertexSize)
        {
            SPDLOG_LOGGER_DEBUG(logger_, "Resizing vertex buffer to {}", vertexSize);
            wgpu::BufferDescriptor vertexBufferDesc {
                .label = "WebGpuUIRendererBackend vertex buffer",
                .usage = wgpu::BufferUsage::Vertex,
                .size = vertexSize,
                .mappedAtCreation = true};
            frame.vertexBuffer = device_.CreateBuffer(&vertexBufferDesc);
            frame.vertexBufferSize = vertexSize;
        }
        if (frame.indexBufferSize == 0 || frame.indexBufferSize < indexSize)
        {
            SPDLOG_LOGGER_DEBUG(logger_, "Resizing index buffer to {}", indexSize);
            wgpu::BufferDescriptor indexBufferDesc {
                .label = "WebGpuUIRendererBackend index buffer",
                .usage = wgpu::BufferUsage::Index,
                .size = indexSize,
                .mappedAtCreation = true};
            frame.indexBuffer = device_.CreateBuffer(&indexBufferDesc);
            frame.indexBufferSize = indexSize;
        }

        uint8_t* vertBufMapped
            = static_cast<uint8_t*>(frame.vertexBuffer.GetMappedRange());
        uint8_t* idxBufMapped
            = static_cast<uint8_t*>(frame.indexBuffer.GetMappedRange());

        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];

            size_t vtxSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
            memcpy(vertBufMapped, cmdList->VtxBuffer.Data, vtxSize);
            vertBufMapped = vertBufMapped + vtxSize;

            size_t idxSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
            memcpy(idxBufMapped, cmdList->IdxBuffer.Data, idxSize);
            idxBufMapped = idxBufMapped + idxSize;
        }

        frame.vertexBuffer.Unmap();
        frame.indexBuffer.Unmap();
    }

    auto passEncoder = wgpuRenderingContext.requestCommandBuffer();

    /* ------------------
     * Setup render state
     *------------------ */

    setupRenderState(passEncoder, frame, drawData, fbWidth, fbHeight);

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
                        passEncoder, frame, drawData, fbWidth, fbHeight);
                else
                    pcmd->UserCallback(cmdList, pcmd);
            }
            else
            {
                passEncoder.SetBindGroup(1, imageBindGroup_);

                /* Project scissor/clipping rectangles into framebuffer space */
                ImVec4 clipRect {
                    (pcmd->ClipRect.x - clipOff.x) * clipScale.x,
                    (pcmd->ClipRect.y - clipOff.y) * clipScale.y,
                    (pcmd->ClipRect.z - clipOff.x) * clipScale.x,
                    (pcmd->ClipRect.w - clipOff.y) * clipScale.y};

                if (clipRect.x < fbWidth && clipRect.y < fbHeight
                    && clipRect.z >= 0.0f && clipRect.w >= 0.0f)
                {
                    /* Negative offsets are illegal */
                    if (clipRect.x < 0.0f)
                        clipRect.x = 0.0f;
                    if (clipRect.y < 0.0f)
                        clipRect.y = 0.0f;

                    passEncoder.SetScissorRect(
                        clipRect.x,
                        clipRect.y,
                        clipRect.z - clipRect.x,
                        clipRect.w - clipRect.y);
                    passEncoder.DrawIndexed(
                        pcmd->ElemCount,
                        1,
                        pcmd->IdxOffset + globalIndexOffset,
                        pcmd->VtxOffset + globalVertexOffset);
                }
            }
        }
        globalVertexOffset += cmdList->VtxBuffer.Size;
        globalIndexOffset += cmdList->IdxBuffer.Size;
    }
}

void WebGpuUIRendererBackend::setupRenderState(
    wgpu::RenderPassEncoder encoder,
    FrameRenderBuffers& frame,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight) const
{
    /* Bind objects to command buffer */
    encoder.SetPipeline(graphicsPipeline_);
    encoder.SetBindGroup(0, commonBindGroup_);

    if (drawData->TotalVtxCount > 0)
    {
        encoder.SetVertexBuffer(0, frame.vertexBuffer, frame.vertexBufferSize);
        encoder.SetIndexBuffer(
            frame.indexBuffer,
            sizeof(ImDrawIdx) == 2 ? wgpu::IndexFormat::Uint16
                                   : wgpu::IndexFormat::Uint32,
            0,
            frame.indexBufferSize);
    }

    /* Setup blend factor */
    wgpu::Color blendColor = {0.f, 0.f, 0.f, 0.f};
    encoder.SetBlendColor(&blendColor);

    /* Viewport */
    encoder.SetViewport(0, 0, fbWidth, fbHeight, 0, 1);

    /* Setup scale and translation through an uniform :
     * The visible imgui space lies from draw_data->DisplayPos (top left) to
     * draw_data->DisplayPos + data_data->DisplaySize (bottom right). DisplayPos is
     * (0,0) for single viewport apps. */
    {
        /* TODO Check
         * https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_dx12.cpp
         */
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
        glm::mat4 mvpMatrix = {
            {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
            {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
            {0.0f, 0.0f, 0.5f, 0.0f},
            {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
        };
        device_.GetDefaultQueue().WriteBuffer(
            uniformBuffer_, 0, &mvpMatrix, sizeof(mvpMatrix));
    }
}

} // namespace webgpu
} // namespace render
} // namespace expengine
