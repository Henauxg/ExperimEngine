#pragma once

#include <unordered_map>

#include <webgpu/webgpu_cpp.h>

#include <engine/render/imgui/UIRendererBackend.hpp>

namespace experim {

class Renderer;

namespace webgpu {

class WebGpuRenderer;
class WgpuTexture;
struct FrameRenderBuffers;

/** Custom back-end inspired by imgui_impl_vulkan. */
class WebGpuUIRendererBackend final : public UIRendererBackend {
public:
    WebGpuUIRendererBackend(
        std::shared_ptr<ImGuiContextWrapper> imguiContext,
        const Renderer& renderer,
        std::shared_ptr<RenderingContext> mainRenderingContext);
    ~WebGpuUIRendererBackend();

    void uploadFonts() override;

    /** Called by ImGui callbacks for secondary viewports.
     * TODO make it shared between rendering backends */
    void uploadBuffersAndDraw(
        ImGuiViewportRendererData* rendererData,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) override;

private:
    /* References */
    const WebGpuRenderer& renderer_;
    const wgpu::Device& device_;

    /* Owned objects */
    wgpu::Sampler fontSampler_;
    wgpu::RenderPipeline graphicsPipeline_;
    wgpu::Buffer uniformBuffer_;
    std::unique_ptr<WgpuTexture> fontTexture_;
    wgpu::BindGroup commonBindGroup_;
    wgpu::BindGroupLayout imageBindGroupLayout_;
    wgpu::BindGroup fontImageBindGroup_;
    std::unordered_map<ImTextureID, wgpu::BindGroup> imageBindGroupsStorage_;

    void setupRenderState(
        wgpu::RenderPassEncoder encoder,
        FrameRenderBuffers& frame,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) const;
};

} // namespace webgpu
} // namespace experim
