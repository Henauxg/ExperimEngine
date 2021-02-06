#include "WGpuUIRendererBackend.hpp"

#include <string>

#include <glm/glm.hpp>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>
#include <engine/render/imgui/lib/imgui_internal.h>
#include <engine/render/imgui/wgpu/spirv/wgpu_imgui_shaders_spirv.h>
#include <engine/render/wgpu/WGpuRenderer.hpp>
#include <engine/render/wgpu/WGpuRenderingContext.hpp>
#include <engine/render/wgpu/resources/WGpuTexture.hpp>

// #define DISABLE_MAPPED_VTX_IDX_BUFFERS 1

namespace {

const std::string RENDERER_BACKEND_NAME = "ExperimEngine_WebGPU_Renderer";

/* No viewports for Emscripten backend */
#ifdef __EMSCRIPTEN__
const bool BACKEND_HAS_VIEWPORTS = false;
#else
const bool BACKEND_HAS_VIEWPORTS = true;
#endif

/* TODO : Should there be any sync in order to reuse buffers (index/vertex) ?
 */
const uint32_t FAKE_SWAPCHAIN_IMAGE_COUNT = 3;

} // namespace

/* TODO : Vertex/Index buffers encapsulation */
/* TODO : Error handling for WebGPU objects */

namespace experim {
namespace webgpu {

struct FrameRenderBuffers {
    wgpu::Buffer vertexBuffer = nullptr;
    wgpu::Buffer indexBuffer = nullptr;
#ifdef DISABLE_MAPPED_VTX_IDX_BUFFERS
    size_t vertexBufferSize = 0;
    size_t indexBufferSize = 0;
#else
    uint32_t vertexDataSize = 0;
    uint32_t indexDataSize = 0;
#endif
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
        auto& frame = renderBuffers_.at(frameIndex_);
        frameIndex_ = (frameIndex_ + 1) % renderBuffers_.size();
        return frame;
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
        .entryCount = static_cast<uint32_t>(commonBindGroupLayoutEntries.size()),
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
        .bindGroupLayoutCount = static_cast<uint32_t>(bindGroupLayouts.size()),
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
           .attributeCount = static_cast<uint32_t>(vertAttributesDesc.size()),
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

    wgpu::ColorStateDescriptor colorState = {
        .format = SWAPCHAIN_TEXTURE_FORMAT, .writeMask = wgpu::ColorWriteMask::All};
    colorState.alphaBlend
        = {.operation = wgpu::BlendOperation::Add,
           .srcFactor = wgpu::BlendFactor::One,
           .dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha};
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
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = sizeof(glm::mat4)};
    uniformBuffer_ = device_.CreateBuffer(&uniformBuffDesc);

    /* Bind groups creation */
    std::array<wgpu::BindGroupEntry, 2> bindGroupEntries {
        wgpu::BindGroupEntry {
            .binding = 0, .buffer = uniformBuffer_, .size = sizeof(glm::mat4)},
        wgpu::BindGroupEntry {.binding = 1, .sampler = fontSampler_}};

    wgpu::BindGroupDescriptor commonBindGroupDesc {
        .layout = bindGroupLayouts[0],
        .entryCount = static_cast<uint32_t>(bindGroupEntries.size()),
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
    fontImageBindGroup_ = device_.CreateBindGroup(&imageBindGroupDesc);
    /* Store the font texture view handle as an ID and store a reference to its
     * BindGroup */
    imageBindGroupsStorage_.emplace(io.Fonts->TexID, fontImageBindGroup_);
}

void WebGpuUIRendererBackend::uploadBuffersAndDraw(
    ImGuiViewportRendererData* rendererData,
    ImDrawData* drawData,
    uint32_t fbWidth,
    uint32_t fbHeight)
{
    auto wgpuViewportData
        = dynamic_cast<WGpuImGuiViewportRendererData*>(rendererData);
    auto& wgpuRenderingContext
        = dynamic_cast<WebGpuRenderingContext&>(*rendererData->renderingContext_);

    /* ------------------
     * Upload to index and vertex buffers
     *------------------ */

    auto& frame = wgpuViewportData->requestFrameRenderBuffers();

#ifdef DISABLE_MAPPED_VTX_IDX_BUFFERS
    if (drawData->TotalVtxCount > 0)
    {
        /* Buffer size must be a multiple of 4
         * See https://gpuweb.github.io/gpuweb/#dom-gpuqueue-writebuffer
         */
        size_t neededVertexDataSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        size_t neededIndexDataSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        /* Deduce buffer size from needed size */
        const uint8_t BUFFER_SIZE_ALIGNMENT = 4;
        size_t vertexBufferSize = neededVertexDataSize + BUFFER_SIZE_ALIGNMENT
            - (neededVertexDataSize % BUFFER_SIZE_ALIGNMENT);
        size_t indexBufferSize = neededIndexDataSize + BUFFER_SIZE_ALIGNMENT
            - (neededIndexDataSize % BUFFER_SIZE_ALIGNMENT);

        if (frame.vertexBuffer == nullptr
            || frame.vertexBufferSize < vertexBufferSize)
        {
            wgpu::BufferDescriptor vertexBufferDesc {
                .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst,
                .size = vertexBufferSize,
                .mappedAtCreation = false};
            frame.vertexBuffer = device_.CreateBuffer(&vertexBufferDesc);
            frame.vertexBufferSize = vertexBufferSize;
        }

        if (frame.indexBuffer == nullptr || frame.indexBufferSize < indexBufferSize)
        {
            wgpu::BufferDescriptor indexBufferDesc {
                .usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst,
                .size = indexBufferSize,
                .mappedAtCreation = false};
            frame.indexBuffer = device_.CreateBuffer(&indexBufferDesc);
            frame.indexBufferSize = indexBufferSize;
        }

        auto localVtxBuff = std::make_unique<uint8_t[]>(vertexBufferSize);
        auto localIdxBuff = std::make_unique<uint8_t[]>(indexBufferSize);

        uint8_t* vtxPtr = localVtxBuff.get();
        uint8_t* idxPtr = localIdxBuff.get();
        for (int n = 0; n < drawData->CmdListsCount; n++)
        {
            const ImDrawList* cmdList = drawData->CmdLists[n];

            size_t vtxSize = cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
            memcpy(vtxPtr, cmdList->VtxBuffer.Data, vtxSize);
            vtxPtr = vtxPtr + vtxSize;

            size_t idxSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
            memcpy(idxPtr, cmdList->IdxBuffer.Data, idxSize);
            idxPtr = idxPtr + idxSize;
        }

        auto queue = device_.GetDefaultQueue();
        queue.WriteBuffer(
            frame.vertexBuffer, 0, localVtxBuff.get(), vertexBufferSize);
        queue.WriteBuffer(frame.indexBuffer, 0, localIdxBuff.get(), indexBufferSize);
    }
#else
    if (drawData->TotalVtxCount > 0)
    {
        /* With mappedAtCreation to true, buffer size must be a multiple of 4
         * See
         * https://gpuweb.github.io/gpuweb/#dom-gpubufferdescriptor-mappedatcreation
         * mappedAtCreation is used here since ->map() seems to be an async call
         * only which is a pain.
         */
        frame.vertexDataSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
        frame.indexDataSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
        const uint8_t BUFFER_SIZE_ALIGNMENT = 4;
        size_t vertexBufferSize = frame.vertexDataSize + BUFFER_SIZE_ALIGNMENT
            - (frame.vertexDataSize % BUFFER_SIZE_ALIGNMENT);
        size_t indexBufferSize = frame.indexDataSize + BUFFER_SIZE_ALIGNMENT
            - (frame.indexDataSize % BUFFER_SIZE_ALIGNMENT);

        /** Force buffers to be re-created : see
         * https://github.com/gfx-rs/wgpu-rs/issues/9
         * TODO : list the other options (less simple, and potentially less
         * optimized)
         */
        wgpu::BufferDescriptor vertexBufferDesc {
            .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopySrc,
            .size = vertexBufferSize,
            .mappedAtCreation = true};
        frame.vertexBuffer = device_.CreateBuffer(&vertexBufferDesc);

        wgpu::BufferDescriptor indexBufferDesc {
            .usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopySrc,
            .size = indexBufferSize,
            .mappedAtCreation = true};
        frame.indexBuffer = device_.CreateBuffer(&indexBufferDesc);

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

            size_t idxSize = cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
            memcpy(idxBufMapped, cmdList->IdxBuffer.Data, idxSize);
            idxBufMapped = idxBufMapped + idxSize;
        }

        frame.vertexBuffer.Unmap();
        frame.indexBuffer.Unmap();
    }
#endif

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
                /* Custom texture binding : try to find the BindGroup associated to
                 * the TexID */
                auto bindGroup = imageBindGroupsStorage_.find(pcmd->TextureId);
                if (bindGroup != imageBindGroupsStorage_.end())
                {
                    /* Use the existing BindGroup */
                    passEncoder.SetBindGroup(1, bindGroup->second);
                }
                else
                {
                    /* Create a new BindGroupd for this texture and store it */
                    wgpu::BindGroupEntry imageBindGroupEntry {
                        .binding = 0,
                        .textureView = (WGPUTextureView) pcmd->TextureId};
                    wgpu::BindGroupDescriptor imageBindGroupDesc {
                        .layout = imageBindGroupLayout_,
                        .entryCount = 1,
                        .entries = &imageBindGroupEntry};
                    auto createdBindGroup
                        = device_.CreateBindGroup(&imageBindGroupDesc);
                    imageBindGroupsStorage_.emplace(
                        pcmd->TextureId, createdBindGroup);

                    passEncoder.SetBindGroup(1, createdBindGroup);
                }

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
                        static_cast<uint32_t>(clipRect.x),
                        static_cast<uint32_t>(clipRect.y),
                        static_cast<uint32_t>(clipRect.z - clipRect.x),
                        static_cast<uint32_t>(clipRect.w - clipRect.y));
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
#ifdef DISABLE_MAPPED_VTX_IDX_BUFFERS
        size_t vtxSize = frame.vertexBufferSize;
        size_t idxSize = frame.vertexBufferSize;
#else
        size_t vtxSize = frame.vertexDataSize;
        size_t idxSize = frame.indexDataSize;
#endif
        encoder.SetVertexBuffer(0, frame.vertexBuffer, 0, vtxSize);
        encoder.SetIndexBuffer(
            frame.indexBuffer,
            sizeof(ImDrawIdx) == 2 ? wgpu::IndexFormat::Uint16
                                   : wgpu::IndexFormat::Uint32,
            0,
            idxSize);
    }

    /* Setup blend factor */
    wgpu::Color blendColor = {0.f, 0.f, 0.f, 0.f};
    encoder.SetBlendColor(&blendColor);

    /* Viewport */
    encoder.SetViewport(
        0, 0, static_cast<float>(fbWidth), static_cast<float>(fbHeight), 0.0f, 1.0f);

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
} // namespace experim
