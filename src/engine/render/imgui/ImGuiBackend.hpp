#pragma once

#include <memory>

#include <engine/render/Renderer.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace spdlog {
class logger;
}

namespace experim {

class RenderingContext;
class Window;
class ImGuiContextWrapper;
class UIPlatformBackendSDL;
class UIRendererBackend;

/** Custom back-end */
class ImguiBackend {
public:
    ImguiBackend(
        const Renderer& renderer,
        std::shared_ptr<RenderingContext> mainRenderingContext,
        std::shared_ptr<Window> mainWindow);
    ~ImguiBackend();

    /* TODO : Could swap ImGuiContext if multiple contexts are used. */
    bool handleEvent(const SDL_Event& event);
    void prepareFrame();
    void renderFrame();

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

} // namespace experim
