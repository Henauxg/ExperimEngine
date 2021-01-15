#include "WGpuRenderingContext.hpp"

#ifdef __EMSCRIPTEN__
#include <webgpu/webgpu_cpp.h>
#endif

#include <engine/log/ExpengineLog.hpp>

namespace {

}

namespace expengine {
namespace render {
namespace webgpu {

WebGpuRenderingContext::WebGpuRenderingContext(
    const wgpu::Device& device,
    std::shared_ptr<Window> window,
    AttachmentsFlags attachmentFlags)
    : RenderingContext()
    , window_(window)
    , device_(device)
    , attachmentsFlags_(attachmentFlags)
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuRenderingContext creation");

    /* Surface creation */
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc {};
    canvasDesc.selector = "#canvas";
    wgpu::SurfaceDescriptor surfDesc {};
    surfDesc.nextInChain = &canvasDesc;
    wgpu::Instance instance {};
    /* A null instaance can be used for CreateSurface */
    surface_ = instance.CreateSurface(&surfDesc);
}

WebGpuRenderingContext::~WebGpuRenderingContext()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuRenderingContext destruction");
}

void WebGpuRenderingContext::handleSurfaceChanges()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

void WebGpuRenderingContext::beginFrame()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

void WebGpuRenderingContext::submitFrame()
{
    /* TODO implement */
    SPDLOG_LOGGER_WARN(logger_, "TODO Implement");
}

std::shared_ptr<RenderingContext> WebGpuRenderingContext::clone(
    std::shared_ptr<Window> window,
    AttachmentsFlags attachmentFlags)
{
    return std::make_shared<WebGpuRenderingContext>(
        device_, window, attachmentFlags);
}

void WebGpuRenderingContext::buildSwapchainObjects(
    std::pair<uint32_t, uint32_t> requestedExtent)
{
    /* Create the swapchain and release the previous one if any */
    wgpu::SwapChainDescriptor swapchainDesc {
        .usage = wgpu::TextureUsage::OutputAttachment,
        .format = wgpu::TextureFormat::BGRA8Unorm,
        .width = requestedExtent.first,
        .height = requestedExtent.second,
        .presentMode = wgpu::PresentMode::Fifo};
    swapchain_ = std::make_unique<wgpu::SwapChain>(
        device_.CreateSwapChain(surface_, &swapchainDesc));
}

} // namespace webgpu
} // namespace render
} // namespace expengine
