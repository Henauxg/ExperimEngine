#pragma once

#include <engine/render/Renderer.hpp>
#include <engine/render/imgui/UIRendererBackend.hpp>

namespace expengine {
namespace render {
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

    void uploadFonts();

    /** Called by ImGui callbacks for secondary viewports.
     * TODO make it shared between rendering backends */
    void renderUI(RenderingContext& renderingContext, ImDrawData* drawData) const;

private:
    /* References */
    const WebGpuRenderer& renderer_;
};

} // namespace webgpu
} // namespace render
} // namespace expengine
