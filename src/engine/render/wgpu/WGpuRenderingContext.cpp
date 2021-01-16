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
    , queue_(device_.GetDefaultQueue())
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

    requestedExtent_ = window_->getDrawableSizeInPixels();
    buildSwapchainObjects(requestedExtent_);
}

WebGpuRenderingContext::~WebGpuRenderingContext()
{
    SPDLOG_LOGGER_DEBUG(logger_, "WebGpuRenderingContext destruction");
}

void WebGpuRenderingContext::handleSurfaceChanges()
{
    auto currentExtent = window_->getDrawableSizeInPixels();
    if (requestedExtent_ != currentExtent)
    {
        requestedExtent_ = currentExtent;
        buildSwapchainObjects(requestedExtent_);
    }
}

void WebGpuRenderingContext::beginFrame()
{
    EXPENGINE_ASSERT(
        !frameToSubmit_, "Error, beginFrame() was called instead of submitFrame()");

    handleSurfaceChanges();

    wgpu::TextureView backbufferView = swapchain_.GetCurrentTextureView();

    wgpu::RenderPassColorAttachmentDescriptor attachment {
        .attachment = backbufferView,
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store,
        /* TODO Better handling of clear color */
        .clearColor = {0, 0, 0, 1}};

    /* Register this frame render pass descriptor */
    renderpass_.colorAttachmentCount = 1;
    renderpass_.colorAttachments = &attachment;

    encoders_.clear();
    passEncoders_.clear();
    frameToSubmit_ = true;
}

void WebGpuRenderingContext::submitFrame()
{
    EXPENGINE_ASSERT(
        frameToSubmit_, "Error, submitFrame() was called instead of beginFrame()");

    std::vector<wgpu::CommandBuffer> commandBuffers;
    for (uint32_t i = 0; i < passEncoders_.size(); i++)
    {
        passEncoders_.at(i).EndPass();
        commandBuffers.push_back(encoders_.at(i).Finish());
    }

    queue_.Submit(commandBuffers.size(), commandBuffers.data());

    frameToSubmit_ = false;
}

wgpu::RenderPassEncoder& WebGpuRenderingContext::requestCommandBuffer()
{
    EXPENGINE_ASSERT(
        frameToSubmit_, "Error, requestCommandBuffer() outside of a frame");

    /* TODO Proper CommandBuffer mechanism */
    encoders_.push_back(device_.CreateCommandEncoder());
    /* Use this frame render pass descriptor */
    passEncoders_.push_back(encoders_.back().BeginRenderPass(&renderpass_));

    return passEncoders_.back();
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
        .format = wgpu::TextureFormat::RGBA8Unorm,
        .width = requestedExtent.first,
        .height = requestedExtent.second,
        .presentMode = wgpu::PresentMode::Fifo};
    swapchain_ = device_.CreateSwapChain(surface_, &swapchainDesc);
}

} // namespace webgpu
} // namespace render
} // namespace expengine
