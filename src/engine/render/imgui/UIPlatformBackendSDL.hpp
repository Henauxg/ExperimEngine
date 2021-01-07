#pragma once

#include <array>
#include <memory>

#include <SDL2\SDL_events.h>

#include <engine/render/imgui/lib/imgui.h>

struct SDL_Cursor;

namespace spdlog {
class logger;
}

namespace expengine {
namespace render {

class Window;
class ImGuiContextWrapper;

/** Custom back-end based on imgui_impl_sdl */
class UIPlatformBackendSDL {
public:
    UIPlatformBackendSDL(
        std::shared_ptr<ImGuiContextWrapper> imguiContext,
        std::shared_ptr<Window> mainWindow);
    ~UIPlatformBackendSDL();

    void eraseClipboardData();
    const char* getClipboardData();
    bool handleEvent(const SDL_Event& event);

private:
    /* ImGui */
    const std::shared_ptr<ImGuiContextWrapper> imguiContext_;

    /* Platform */
    char* clipboardTextData_;
    bool mouseCanUseGlobalState_;
    std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> mouseCursors_;
    std::array<bool, 3> mousePressed_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
