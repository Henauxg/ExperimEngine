#pragma once

#include <engine/render/RenderingContext.hpp>
#include <engine/render/imgui/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

class RenderingContext;

class UIRendererBackend {
public:
    virtual ~UIRendererBackend();

    virtual void uploadFonts() = 0;
    virtual void renderUI(RenderingContext& renderingContext, ImDrawData* drawData)
        const = 0;

protected:
    UIRendererBackend(
        std::shared_ptr<ImGuiContextWrapper> imguiContext,
        std::shared_ptr<RenderingContext> mainRenderingContext,
        const std::string& renderingBackendName,
        bool hasVtxOffset = true,
        bool hasViewports = true);

    /* ImGui */
    const std::shared_ptr<ImGuiContextWrapper> imguiContext_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
