#pragma once

#include <memory>

#include <engine/render/Renderer.hpp>
#include <engine/render/RenderingContext.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/UIPlatformBackendSDL.hpp>
#include <engine/render/imgui/UIRendererBackend.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

class RenderingContext;

/** Custom back-end */
class ImguiBackend {
public:
    ImguiBackend(
        const Renderer& renderer,
        std::shared_ptr<RenderingContext> mainRenderingContext,
        std::shared_ptr<Window> mainWindow);
    ~ImguiBackend();

    /* TODO : Could swap ImGuiContext if multiple contexts are used. */
    inline bool handleEvent(const SDL_Event& event)
    {
        return platformBackend_->handleEvent(event);
    };

private:
    /* ImGui */
    std::shared_ptr<ImGuiContextWrapper> imguiContext_;
    ImFont* fontRegular_;

    /* Platform */
    std::unique_ptr<UIPlatformBackendSDL> platformBackend_;
    /* Rendering */
    std::unique_ptr<UIRendererBackend> renderingBackend_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
