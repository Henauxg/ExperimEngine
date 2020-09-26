#pragma once

#include <array>

#include <SDL2\SDL_events.h>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/impl/ImguiContext.hpp>
#include <engine/render/imgui/lib/imgui.h>

struct SDL_Cursor;

namespace expengine {
namespace render {

/** Custom back-end based on imgui_impl_sdl */
class PlatformBackendSDL {
public:
	PlatformBackendSDL::PlatformBackendSDL(
		std::shared_ptr<ImguiContext> context,
		std::shared_ptr<Window> window);
	~PlatformBackendSDL();

	void eraseClipboardData();
	const char* getClipboardData();
	bool handleEvent(const SDL_Event& event);

private:
	/* ImGui */
	const std::shared_ptr<ImguiContext> context_;

	/* Platform */
	char* clipboardTextData_;
	bool mouseCanUseGlobalState_;
	SDL_Cursor* mouseCursors_[ImGuiMouseCursor_COUNT];
	std::array<bool, 3> mousePressed_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
