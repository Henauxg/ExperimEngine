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

    /* Create ImGui shaders modules */
    wgpu::ShaderModule vertexShaderModule {};
    {
        wgpu::ShaderModuleSPIRVDescriptor spirvDesc {};
        spirvDesc.codeSize = sizeof(__glsl_wgpu_shader_vert_spv);
        spirvDesc.code = (uint32_t*) __glsl_wgpu_shader_vert_spv;

        wgpu::ShaderModuleDescriptor descriptor {.nextInChain = &spirvDesc};
        vertexShaderModule = device_.CreateShaderModule(&descriptor);
    }
    wgpu::ShaderModule fragmentShaderModule {};
    {
        wgpu::ShaderModuleSPIRVDescriptor spirvDesc {};
        spirvDesc.codeSize = sizeof(__glsl_wgpu_shader_vert_spv);
        spirvDesc.code = (uint32_t*) __glsl_wgpu_shader_vert_spv;

        wgpu::ShaderModuleDescriptor descriptor {.nextInChain = &spirvDesc};
        fragmentShaderModule = device_.CreateShaderModule(&descriptor);
    }

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

    /* TODO End RenderPass */

    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

} // namespace webgpu
} // namespace render
} // namespace expengine
