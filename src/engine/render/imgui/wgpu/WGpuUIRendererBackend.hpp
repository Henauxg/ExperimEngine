#pragma once

#include <engine/render/imgui/UIRendererBackend.hpp>

namespace expengine {
namespace render {

class Renderer;

namespace webgpu {

class WebGpuRenderer;

/** Custom back-end inspired by imgui_impl_vulkan. */
class WebGpuUIRendererBackend : public UIRendererBackend {
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
        RenderingContext& renderingContext,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) const override;

private:
    /* References */
    const WebGpuRenderer& renderer_;
};

} // namespace webgpu
} // namespace render
} // namespace expengine
