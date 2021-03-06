#pragma once

#include <webgpu/webgpu_cpp.h>

#include <engine/render/RenderingContext.hpp>
#include <engine/render/Window.hpp>

namespace experim {
namespace webgpu {

/* RGBA8Unorm seems to be deprecated for the swapchain (chrome debugger) */
const wgpu::TextureFormat SWAPCHAIN_TEXTURE_FORMAT = wgpu::TextureFormat::BGRA8Unorm;

class WebGpuRenderingContext final : public RenderingContext {
public:
    WebGpuRenderingContext(
        const wgpu::Device& device,
        std::shared_ptr<Window> window,
        AttachmentsFlags attachmentFlags);
    ~WebGpuRenderingContext() override;

    /* Accessors */
    inline const Window& window() const override { return *window_; };

    /* Call to make the RenderingContext check its surface and adapt its objects to
     * it. */
    void handleSurfaceChanges() override;

    /* Frame rendering */
    virtual void beginFrame() override;
    virtual void submitFrame() override;
    /* TODO : should have a common buffer interfaces between backends */
    wgpu::RenderPassEncoder& requestCommandBuffer();

    std::shared_ptr<RenderingContext> clone(
        std::shared_ptr<Window> window,
        AttachmentsFlags attachmentFlags) override;

private:
    /* References */
    const wgpu::Device& device_;

    /* Configuration */
    AttachmentsFlags attachmentsFlags_;

    /* Owned objects */
    wgpu::Queue queue_;
    std::shared_ptr<const Window> window_;
    wgpu::Surface surface_;
    wgpu::SwapChain swapchain_;
    std::pair<uint32_t, uint32_t> requestedExtent_;

    /* Frames */
    /* Current texture, pass and attachment */
    wgpu::TextureView backbufferView_;
    wgpu::RenderPassColorAttachmentDescriptor colorAttachment_;
    wgpu::RenderPassDescriptor renderpass_;
    /* Command buffer objects */
    std::vector<wgpu::CommandEncoder> encoders_;
    std::vector<wgpu::RenderPassEncoder> passEncoders_;

    void buildSwapchainObjects(std::pair<uint32_t, uint32_t> requestedExtent);
};

} // namespace webgpu
} // namespace experim
