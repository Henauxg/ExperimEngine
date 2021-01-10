#pragma once

#include <functional>
#include <memory>
#include <string>

#include <engine/render/imgui/ImGuiViewportRendererData.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace spdlog {
class logger;
}

namespace expengine {
namespace render {

class ImGuiContextWrapper;
class RenderingContext;

class UIRendererBackend {
public:
    virtual ~UIRendererBackend();

    virtual void uploadFonts() = 0;
    virtual void renderUI(
        ImGuiViewportRendererData* renderData,
        ImDrawData* drawData,
        uint32_t fbWidth,
        uint32_t fbHeight) const = 0;

protected:
    UIRendererBackend(
        std::shared_ptr<ImGuiContextWrapper> imguiContext,
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
